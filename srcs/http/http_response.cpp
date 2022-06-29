#include "http/http_response.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <vector>

#include "http/file_event_handler.hpp"
#include "http/http_constants.hpp"
#include "http/http_request.hpp"
#include "server/epoll.hpp"
#include "utils/io.hpp"
#include "utils/string.hpp"

namespace http {

const std::string HttpResponse::kDefaultHttpVersion = "HTTP/1.1";

HttpResponse::HttpResponse()
    : http_version_(kDefaultHttpVersion),
      location_(NULL),
      epoll_(NULL),
      file_buffer_(NULL),
      file_fde_(NULL),
      writtern_status_headers_count_(0) {}

HttpResponse::HttpResponse(const config::LocationConf *location,
                           server::Epoll *epoll)
    : http_version_(kDefaultHttpVersion),
      location_(location),
      epoll_(epoll),
      file_buffer_(NULL),
      file_fde_(NULL),
      writtern_status_headers_count_(0) {}

HttpResponse::HttpResponse(const HttpResponse &rhs) {
  *this = rhs;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &rhs) {
  if (this != &rhs) {
    http_version_ = rhs.http_version_;
    status_ = rhs.status_;
    status_message_ = rhs.status_message_;
    headers_ = rhs.headers_;
    body_bytes_ = rhs.body_bytes_;
  }
  return *this;
}

HttpResponse::~HttpResponse() {
  if (file_buffer_) {
    if (file_buffer_->is_unregistered) {
      delete file_fde_;
      delete file_buffer_;
    } else {
      epoll_->Unregister(file_fde_);
      file_buffer_->is_unregistered = true;
    }
  }
}

Result<void> HttpResponse::RegisterFile(std::string file_path) {
  FileBuffer *file_buffer = new FileBuffer();
  int flags;
  if (!utils::IsRegularFile(file_path) || !utils::IsReadableFile(file_path) ||
      (file_buffer->file_fd = open(file_path.c_str(), O_RDONLY)) < 0 ||
      (flags = fcntl(file_buffer->file_fd, F_GETFL, 0)) < 0 ||
      fcntl(file_buffer->file_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    delete file_buffer;
    return Error();
  }
  file_buffer->events = 0;
  file_buffer->is_eof = false;
  file_buffer->is_unregistered = false;

  server::FdEvent *fde = server::CreateFdEvent(file_buffer_->file_fd,
                                               HandleFileEvent, file_buffer);
  epoll_->Register(fde);
  epoll_->Add(fde, server::kFdeRead);

  file_buffer_ = file_buffer;
  file_fde_ = fde;
}

//========================================================================
// Writer

Result<void> HttpResponse::Write(int fd) {
  Result<int> status_header_res = WriteStatusAndHeader(fd);
  if (status_header_res.IsErr()) {
    return status_header_res.Err();
  }
  if (status_header_res.IsOk() && status_header_res.Ok() > 0) {
    return Result<void>();
  }
  if (!body_bytes_.empty()) {
    int write_res =
        write(fd, body_bytes_.data(), body_bytes_.size() - written_body_count_);
    if (write_res < 0) {
      return Error();
    }
    written_body_count_ += write_res;
  }
}

Result<int> HttpResponse::WriteStatusAndHeader(int fd) {
  if (IsStatusAndHeadersWritingCompleted()) {
    return 0;
  }
  if (status_and_headers_bytes_.empty()) {
    status_and_headers_bytes_ = SerializeStatusAndHeader();
    writtern_status_headers_count_ = 0;
  }

  int write_res = write(
      fd, status_and_headers_bytes_.data() + writtern_status_headers_count_,
      status_and_headers_bytes_.size() - writtern_status_headers_count_);
  if (write_res < 0) {
    return Error();
  }
  writtern_status_headers_count_ += write_res;
  // この関数内のwrite後に呼び出し元でもwriteがノンブロッキングで可能かは保証されていない
  return write_res;
}

//========================================================================
// Reponse Maker

void HttpResponse::MakeResponse(server::ConnSocket *conn_sock) {}

bool HttpResponse::MakeErrorResponse(const config::LocationConf *location,
                                     const HttpRequest &request,
                                     HttpStatus status) {
  SetStatus(status, StatusCodes::GetMessage(status));

  const std::map<http::HttpStatus, std::string> &error_pages =
      location->GetErrorPages();
  if (error_pages.find(status) == error_pages.end() ||
      RegisterFile(error_pages.at(status)).IsErr()) {
    body_bytes_ = MakeErrorResponseBody(status);
  }
  return true;
}

std::string HttpResponse::MakeErrorResponseBody(HttpStatus status) {
  std::stringstream ss;
  ss << status;
  ss << " ";
  ss << StatusCodes::GetMessage(status);
  std::string status_with_msg = ss.str();

  std::string head =
      "<html>"
      "<head><title>" +
      status_with_msg + "</title></head><body>";
  std::string body = "<h1>" + status_with_msg + "</h1>";
  std::string tail = "</body></html>";
  return head + body + tail;
}

//========================================================================
// Status checker

bool HttpResponse::IsReadyToWrite() {
  // TODO: 実装する
}

bool HttpResponse::IsAllDataWritingCompleted() {
  // TODO: 実装する
}

bool HttpResponse::IsStatusAndHeadersWritingCompleted() {
  return !status_and_headers_bytes_.empty() &&
         writtern_status_headers_count_ == status_and_headers_bytes_.size();
}

//========================================================================
// Serialization

utils::ByteVector HttpResponse::SerializeStatusAndHeader() {
  utils::ByteVector bytes;
  utils::ByteVector status_line = SerializeStatusLine();
  bytes.insert(bytes.end(), status_line.begin(), status_line.end());
  utils::ByteVector header_lines = SerializeHeaders();
  bytes.insert(bytes.end(), header_lines.begin(), header_lines.end());
  return bytes;
}

utils::ByteVector HttpResponse::SerializeStatusLine() const {
  std::string status_line;
  status_line += http_version_ + " ";
  status_line += status_ + " ";
  status_line += status_message_ + http::kCrlf;
  return status_line;
}

utils::ByteVector HttpResponse::SerializeHeaders() const {
  std::string header_lines;
  for (HeaderMap::const_iterator headers_it = headers_.begin();
       headers_it != headers_.end(); ++headers_it) {
    typedef HeaderMap::mapped_type HeaderValuesType;
    HeaderValuesType header_values = headers_it->second;
    for (HeaderValuesType::const_iterator value_it = header_values.begin();
         value_it != header_values.end(); ++value_it) {
      const std::string header = headers_it->first + ": " + *value_it;
      header_lines += header + http::kCrlf;
    }
  }
  return header_lines;
}

//========================================================================
// Setter, Getter

void HttpResponse::SetHttpVersion(const std::string &http_version) {
  http_version_ = http_version;
}

void HttpResponse::SetStatus(HttpStatus status) {
  status_ = status;
  status_message_ = StatusCodes::GetMessage(status);
}

void HttpResponse::SetStatus(HttpStatus status,
                             const std::string &status_message) {
  status_ = status;
  status_message_ = status_message;
}

void HttpResponse::SetStatusMessage(const std::string &status_message) {
  status_message_ = status_message;
}

void HttpResponse::AppendHeader(const std::string &header,
                                const std::string &value) {
  headers_[header].push_back(value);
}

}  // namespace http

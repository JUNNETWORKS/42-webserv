#include "http/http_response.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <vector>

#include "http/http_constants.hpp"
#include "http/http_request.hpp"
#include "server/epoll.hpp"
#include "utils/io.hpp"
#include "utils/string.hpp"

namespace http {

const std::string HttpResponse::kDefaultHttpVersion = "HTTP/1.1";

HttpResponse::HttpResponse(const config::LocationConf *location,
                           server::Epoll *epoll)
    : location_(location),
      epoll_(epoll),
      http_version_(kDefaultHttpVersion),
      status_(OK),
      status_message_(StatusCodes::GetMessage(OK)),
      headers_(),
      status_and_headers_bytes_(),
      writtern_status_headers_count_(0),
      body_bytes_(),
      written_body_count_(0),
      file_fd_(-1),
      is_file_eof_(false) {
  assert(epoll_ != NULL);
}

HttpResponse::~HttpResponse() {
  if (file_fd_ >= 0) {
    close(file_fd_);
  }
}

Result<void> HttpResponse::RegisterFile(std::string file_path) {
  if (!utils::IsRegularFile(file_path) || !utils::IsReadableFile(file_path) ||
      (file_fd_ = open(file_path.c_str(), O_RDONLY)) < 0) {
    return Error();
  }
  printf("RegisterFile(%s)\n", file_path.c_str());
  return Result<void>();
}

//========================================================================
// Writer

Result<void> HttpResponse::Write(int fd) {
  Result<int> status_header_res = WriteStatusAndHeader(fd);
  if (status_header_res.IsErr()) {
    return status_header_res.Err();
  }
  if (status_header_res.Ok() > 0) {
    return Result<void>();
  }

  if (file_fd_ >= 0 && !is_file_eof_) {
    utils::Byte buf[kBytesPerRead];
    int read_res = read(file_fd_, buf, kBytesPerRead);
    if (read_res < 0) {
      return Error();
    } else if (read_res == 0) {
      is_file_eof_ = true;
    }
    body_bytes_.AppendDataToBuffer(buf, read_res);
  }
  if (!body_bytes_.empty()) {
    int write_res = write(fd, body_bytes_.data() + written_body_count_,
                          body_bytes_.size() - written_body_count_);
    if (write_res < 0) {
      return Error();
    }
    written_body_count_ += write_res;
  }
  return Result<void>();
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

void HttpResponse::MakeResponse(server::ConnSocket *conn_sock) {
  (void)conn_sock;
}

void HttpResponse::MakeErrorResponse(const HttpRequest &request,
                                     HttpStatus status) {
  (void)request;
  // エラーレスポンスを作る前にメンバー変数を初期化したほうがいいかも?
  SetStatus(status, StatusCodes::GetMessage(status));

  if (location_) {
    const std::map<http::HttpStatus, std::string> &error_pages =
        location_->GetErrorPages();
    if (error_pages.find(status) == error_pages.end() ||
        RegisterFile(error_pages.at(status)).IsErr()) {
      body_bytes_ = MakeErrorResponseBody(status);
    }
  } else {
    body_bytes_ = MakeErrorResponseBody(status);
  }
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
  if (file_fd_ >= 0) {
    return !IsStatusAndHeadersWritingCompleted() || !is_file_eof_;
  } else {
    return !IsStatusAndHeadersWritingCompleted() ||
           written_body_count_ < body_bytes_.size();
  }
}

bool HttpResponse::IsAllDataWritingCompleted() {
  if (file_fd_ >= 0) {
    return IsStatusAndHeadersWritingCompleted() && is_file_eof_ &&
           written_body_count_ == body_bytes_.size();
  } else {
    return IsStatusAndHeadersWritingCompleted() &&
           written_body_count_ == body_bytes_.size();
  }
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
  bytes.insert(bytes.end(), kCrlf.begin(), kCrlf.end());
  return bytes;
}

utils::ByteVector HttpResponse::SerializeStatusLine() const {
  std::stringstream ss;
  ss << http_version_ << " ";
  ss << status_ << " ";
  ss << status_message_ << http::kCrlf;
  return ss.str();
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

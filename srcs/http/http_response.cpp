#include "http/http_response.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <vector>

#include "cgi/cgi_request.hpp"
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
      phase_(kStatusAndHeader),
      http_version_(kDefaultHttpVersion),
      status_(OK),
      status_message_(StatusCodes::GetMessage(OK)),
      headers_(),
      status_and_headers_bytes_(),
      body_bytes_(),
      write_buffer_(),
      file_fd_(-1),
      is_file_eof_(false) {
  assert(epoll_ != NULL);
}

HttpResponse::~HttpResponse() {
  if (file_fd_ >= 0) {
    close(file_fd_);
  }
}

Result<void> HttpResponse::RegisterFile(const std::string &file_path) {
  if (!utils::IsRegularFile(file_path) || !utils::IsReadableFile(file_path)) {
    return Error();
  }
  if ((file_fd_ = open(file_path.c_str(), O_RDONLY)) < 0) {
    return Error();
  }
  unsigned long file_size = utils::GetFileSize(file_path);
  SetHeader("Content-Length", utils::ConvertToStr(file_size));
  return Result<void>();
}

//========================================================================
// Writer

Result<void> HttpResponse::Write(int fd) {
  Result<ssize_t> status_header_res = WriteStatusAndHeader(fd);
  if (status_header_res.IsErr()) {
    return status_header_res.Err();
  }
  if (status_header_res.Ok() > 0) {
    // WriteStatusAndHeader()
    // で書き込みが行われた後にノンブロッキングで書き込める保証はない
    return Result<void>();
  }

  if (!body_bytes_.empty()) {
    Result<ssize_t> result = WriteBody(fd);
    if (result.IsErr()) {
      return result.Err();
    }
  }
  return Result<void>();
}

Result<ssize_t> HttpResponse::WriteStatusAndHeader(int fd) {
  if (phase_ != kStatusAndHeader) {
    return 0;
  }
  if (status_and_headers_bytes_.empty()) {
    status_and_headers_bytes_ = SerializeStatusAndHeader();
  }

  ssize_t write_res = write(fd, status_and_headers_bytes_.data(),
                            status_and_headers_bytes_.size());
  if (write_res < 0) {
    return Error();
  }
  status_and_headers_bytes_.EraseHead(write_res);
  if (status_and_headers_bytes_.empty()) {
    phase_ = kBody;
  }
  // この関数内のwrite後に呼び出し元でもwriteがノンブロッキングで可能かは保証されていない
  return write_res;
}

Result<ssize_t> HttpResponse::ReadFile() {
  if (is_file_eof_) {
    return 0;
  }
  utils::Byte buf[kBytesPerRead];
  ssize_t read_res = read(file_fd_, buf, kBytesPerRead);
  if (read_res < 0) {
    return Error();
  } else if (read_res == 0) {
    is_file_eof_ = true;
  } else {
    body_bytes_.AppendDataToBuffer(buf, read_res);
  }
  return read_res;
}

Result<ssize_t> HttpResponse::WriteBody(int fd) {
  ssize_t write_res = write(fd, body_bytes_.data(), body_bytes_.size());
  if (write_res < 0) {
    return Error();
  }
  body_bytes_.EraseHead(write_res);
  return write_res;
}

//========================================================================
// Reponse Maker

void HttpResponse::MakeResponse(server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();

  if (IsRequestHasConnectionClose(request)) {
    SetHeader("Connection", "close");
  }

  const std::string &abs_file_path =
      location_->GetAbsolutePath(request.GetPath());
  printf("abs_path: %s\n", abs_file_path.c_str());
  if (!utils::IsFileExist(abs_file_path)) {
    MakeErrorResponse(request, NOT_FOUND);
    return;
  }

  if (utils::IsDir(abs_file_path)) {
    body_bytes_ = MakeAutoIndex(abs_file_path, request.GetPath());
    SetHeader("Content-Length", utils::ConvertToStr(body_bytes_.size()));
    SetStatus(OK, StatusCodes::GetMessage(OK));
    AppendHeader("Content-Type", "text/html");
    return;
  }

  if (!utils::IsReadableFile(abs_file_path)) {
    MakeErrorResponse(request, FORBIDDEN);
    return;
  }

  SetStatus(OK, StatusCodes::GetMessage(OK));
  AppendHeader("Content-Type", "text/plain");
  RegisterFile(abs_file_path);
}

Result<void> HttpResponse::PrepareToWrite(server::ConnSocket *conn_sock) {
  (void)conn_sock;
  if (file_fd_ >= 0 && !is_file_eof_) {
    Result<ssize_t> result = ReadFile();
    if (result.IsErr()) {
      return result.Err();
    }
  }
  return Result<void>();
}

void HttpResponse::MakeErrorResponse(const HttpRequest &request,
                                     HttpStatus status) {
  (void)request;
  // エラーレスポンスを作る前にメンバー変数を初期化したほうがいいかも?
  SetStatus(status, StatusCodes::GetMessage(status));
  SetHeader("Connection", "close");

  if (!location_) {
    body_bytes_ = MakeErrorResponseBody(status);
    SetHeader("Content-Length", utils::ConvertToStr(body_bytes_.size()));
    return;
  }

  const std::map<http::HttpStatus, std::string> &error_pages =
      location_->GetErrorPages();
  if (error_pages.find(status) == error_pages.end() ||
      RegisterFile(error_pages.at(status)).IsErr()) {
    body_bytes_ = MakeErrorResponseBody(status);
    SetHeader("Content-Length", utils::ConvertToStr(body_bytes_.size()));
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
  return IsReadyToWriteBody() || IsReadyToWriteFile();
}

bool HttpResponse::IsReadyToWriteBody() {
  return phase_ == kStatusAndHeader ||
         (phase_ == kBody && !body_bytes_.empty());
}

bool HttpResponse::IsReadyToWriteFile() {
  if (file_fd_ >= 0) {
    return phase_ == kStatusAndHeader ||
           (phase_ == kBody && (!is_file_eof_ || !body_bytes_.empty()));
  }
  return false;
}

bool HttpResponse::IsAllDataWritingCompleted() {
  if (file_fd_ >= 0) {
    return phase_ != kStatusAndHeader && is_file_eof_ && body_bytes_.empty();
  } else {
    return phase_ != kStatusAndHeader && body_bytes_.empty();
  }
}

//========================================================================
// Serialization

utils::ByteVector HttpResponse::SerializeStatusAndHeader() const {
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

void HttpResponse::SetHeader(const std::string &header,
                             const std::string &value) {
  headers_[header].clear();
  headers_[header].push_back(value);
}

void HttpResponse::AppendHeader(const std::string &header,
                                const std::string &value) {
  headers_[header].push_back(value);
}

const std::vector<std::string> &HttpResponse::GetHeader(
    const std::string &header) {
  return headers_[header];
}

bool HttpResponse::IsRequestHasConnectionClose(HttpRequest &request) {
  Result<const http::HeaderMap::mapped_type &> header_res =
      request.GetHeader("Connection");
  if (header_res.IsErr())
    return false;
  const http::HeaderMap::mapped_type &header = header_res.Ok();

  return std::find(header.begin(), header.end(), "close") != header.end();
}

}  // namespace http

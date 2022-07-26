#include "http/http_response.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <ctime>
#include <vector>

#include "cgi/cgi_request.hpp"
#include "http/content_types.hpp"
#include "http/http_constants.hpp"
#include "http/http_request.hpp"
#include "server/epoll.hpp"
#include "utils/io.hpp"
#include "utils/path.hpp"
#include "utils/string.hpp"

namespace http {

namespace {
std::string GetTimeStamp();
bool AppendBytesToFile(const std::string &path, const utils::ByteVector &bytes);
}  // namespace
const std::string HttpResponse::kDefaultHttpVersion = "HTTP/1.1";

HttpResponse::HttpResponse(const config::LocationConf *location,
                           server::Epoll *epoll)
    : location_(location),
      epoll_(epoll),
      phase_(kExecuteRequest),
      http_version_(kDefaultHttpVersion),
      status_(OK),
      status_message_(StatusCodes::GetMessage(OK)),
      headers_(),
      write_buffer_(),
      file_fd_(-1) {
  assert(epoll_ != NULL);
}

HttpResponse::HttpResponse(const config::LocationConf *location,
                           server::Epoll *epoll, const HttpStatus status)
    : location_(location),
      epoll_(epoll),
      phase_(kExecuteRequest),
      http_version_(kDefaultHttpVersion),
      status_(OK),
      status_message_(StatusCodes::GetMessage(OK)),
      headers_(),
      write_buffer_(),
      file_fd_(-1) {
  assert(status >= 400);
  phase_ = MakeErrorResponse(status);
  assert(phase_ == kComplete);
}

HttpResponse::~HttpResponse() {
  if (file_fd_ >= 0) {
    close(file_fd_);
  }
}

Result<void> HttpResponse::RegisterFile(const std::string &file_path) {
  if (!utils::IsReadableFile(file_path)) {
    return Error();
  }
  Result<bool> is_regular_file = utils::IsRegularFile(file_path);
  if (is_regular_file.IsErr() || is_regular_file.Ok() == false) {
    return Error();
  }
  if (file_fd_ >= 0) {
    close(file_fd_);
  }
  if ((file_fd_ = open(file_path.c_str(), O_RDONLY | O_CLOEXEC)) < 0) {
    return Error();
  }
  Result<unsigned long> file_size = utils::GetFileSize(file_path);
  if (file_size.IsErr()) {
    close(file_fd_);
    file_fd_ = -1;
    return Error();
  }
  SetHeader("Content-Length", utils::ConvertToStr(file_size.Ok()));
  return Result<void>();
}

//========================================================================
// Writer

Result<void> HttpResponse::WriteToSocket(const int fd) {
  ssize_t write_size = write_buffer_.size() < kWriteMaxSize
                           ? write_buffer_.size()
                           : kWriteMaxSize;
  if (write_size == 0)
    return Result<void>();
  ssize_t write_res = write(fd, write_buffer_.data(), write_size);
  if (write_res < 0)
    return Error();
  write_buffer_.EraseHead(write_size);
  return Result<void>();
}

Result<bool> HttpResponse::ReadFile() {
  utils::Byte buf[kBytesPerRead];
  ssize_t read_res = read(file_fd_, buf, kBytesPerRead);
  if (read_res < 0) {
    return Error();
  } else if (read_res == 0) {
    return true;
  } else {
    write_buffer_.AppendDataToBuffer(buf, read_res);
    return false;
  }
}

//========================================================================
// Reponse Maker

HttpResponse::CreateResponsePhase HttpResponse::ExecuteGetRequest(
    const http::HttpRequest &request) {
  if (IsRequestHasConnectionClose(request)) {
    SetHeader("Connection", "close");
  }

  if (!location_->GetRedirectUrl().empty()) {
    return MakeRedirectResponse();
  }

  std::string abs_file_path = location_->GetAbsolutePath(request.GetPath());
  printf("abs_path: %s\n", abs_file_path.c_str());

  if (!utils::IsFileExist(abs_file_path)) {
    return MakeErrorResponse(NOT_FOUND);
  }

  Result<bool> is_dir_res = utils::IsDir(abs_file_path);
  if (is_dir_res.IsErr()) {
    return MakeErrorResponse(SERVER_ERROR);
  }
  bool is_dir = is_dir_res.Ok();

  if (is_dir) {
    Result<std::string> responsable_index_result =
        GetResponsableIndexPagePath();
    if (responsable_index_result.IsOk()) {
      abs_file_path = responsable_index_result.Ok();
      is_dir_res = utils::IsDir(abs_file_path);
      if (is_dir_res.IsErr()) {
        return MakeErrorResponse(SERVER_ERROR);
      }
      is_dir = is_dir_res.Ok();
    }
  }

  if (!utils::IsFileExist(abs_file_path) ||
      (is_dir && !location_->GetAutoIndex())) {
    return MakeErrorResponse(NOT_FOUND);
  }

  if (is_dir) {
    return MakeAutoIndexResponse(abs_file_path, request.GetPath());
  }

  if (!utils::IsReadableFile(abs_file_path)) {
    return MakeErrorResponse(FORBIDDEN);
  }

  SetStatus(OK, StatusCodes::GetMessage(OK));
  SetHeader("Content-Type", ContentTypes::GetContentTypeFromExt(
                                utils::GetExetension(abs_file_path)));
  Result<void> register_res = RegisterFile(abs_file_path);
  if (register_res.IsErr())
    return MakeErrorResponse(SERVER_ERROR);
  else
    return kStatusAndHeader;
}

HttpResponse::CreateResponsePhase HttpResponse::ExecutePostRequest(
    const server::ConnSocket *conn_sock, const http::HttpRequest &request) {
  std::string request_path = location_->GetAbsolutePath(request.GetPath());

  Result<bool> is_dir_res = utils::IsDir(request_path);
  if (is_dir_res.IsErr()) {
    return MakeErrorResponse(SERVER_ERROR);
  }
  bool is_dir = is_dir_res.Ok();
  std::string target =
      is_dir ? utils::JoinPath(request_path, GetTimeStamp()) : request_path;

  printf("post_path: %s\n", target.c_str());

  HttpStatus response_status = utils::IsFileExist(target) ? OK : CREATED;

  if (AppendBytesToFile(target, request.GetBody()) == false) {
    return MakeErrorResponse(SERVER_ERROR);
  }

  SetStatus(response_status, StatusCodes::GetMessage(response_status));

  if (response_status == CREATED) {
    SetHeader("Location", CreateResourceUrl(target, conn_sock, request));
  }

  SetHeader("Content-Type",
            ContentTypes::GetContentTypeFromExt(utils::GetExetension(target)));

  if (RegisterFile(target).IsErr()) {
    if (response_status == CREATED)
      remove(target.c_str());
    return MakeErrorResponse(SERVER_ERROR);
  }
  return kStatusAndHeader;
}

std::string HttpResponse::CreateResourceUrl(const std::string &local_path,
                                            const server::ConnSocket *conn_sock,
                                            const http::HttpRequest &request) {
  const std::string protocol = "http://";

  std::string host;
  if (request.GetHeader("Host").IsOk()) {
    host = request.GetHeader("Host").Ok()[0];
  } else {
    host = conn_sock->GetServerIp() + ":" + conn_sock->GetServerPort();
  }

  std::string path = local_path;
  path.replace(0, location_->GetRootDir().length(), "");

  return protocol + host + path;
}

HttpResponse::CreateResponsePhase HttpResponse::ExecuteDeleteRequest(
    const http::HttpRequest &request) {
  std::string path = location_->GetAbsolutePath(request.GetPath());

  Result<bool> is_dir_res = utils::IsDir(path);
  if (is_dir_res.IsErr()) {
    return MakeErrorResponse(SERVER_ERROR);
  }
  bool is_dir = is_dir_res.Ok();

  if (is_dir)
    return MakeErrorResponse(BAD_REQUEST);

  if (utils::IsFileExist(path) == false)
    return MakeErrorResponse(NOT_FOUND);

  if (remove(path.c_str()) < 0)
    return MakeErrorResponse(SERVER_ERROR);

  SetStatus(OK, StatusCodes::GetMessage(OK));
  return kStatusAndHeader;
}

HttpResponse::CreateResponsePhase HttpResponse::ExecuteRequest(
    server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();
  const std::string method = request.GetMethod();

  if (method == method_strs::kGet)
    return ExecuteGetRequest(request);
  else if (method == method_strs::kPost)
    return ExecutePostRequest(conn_sock, request);
  else if (method == method_strs::kDelete)
    return ExecuteDeleteRequest(request);
  else
    assert(false);
}

Result<HttpResponse::CreateResponsePhase> HttpResponse::MakeResponseBody() {
  if (file_fd_ < 0)
    return kComplete;
  Result<bool> result = ReadFile();
  if (result.IsErr()) {
    return result.Err();
  } else {
    return result.Ok() ? kComplete : kBody;
  }
}

Result<void> HttpResponse::PrepareToWrite(server::ConnSocket *conn_sock) {
  if (phase_ == kExecuteRequest) {
    phase_ = ExecuteRequest(conn_sock);
  }
  if (phase_ == kStatusAndHeader) {
    write_buffer_.AppendDataToBuffer(SerializeStatusAndHeader());
    phase_ = kBody;
  }
  if (phase_ == kBody) {
    Result<HttpResponse::CreateResponsePhase> body_result = MakeResponseBody();
    if (body_result.IsErr())
      return body_result.Err();
    phase_ = body_result.Ok();
  }
  return Result<void>();
}

HttpResponse::CreateResponsePhase HttpResponse::MakeResponse(
    const std::string &body) {
  write_buffer_.clear();
  if (body.empty() == false)
    SetHeader("Content-Length", utils::ConvertToStr(body.size()));
  write_buffer_.AppendDataToBuffer(SerializeStatusAndHeader());
  write_buffer_.AppendDataToBuffer(body);
  return kComplete;
}

HttpResponse::CreateResponsePhase HttpResponse::MakeRedirectResponse() {
  write_buffer_.clear();
  SetStatus(FOUND);
  SetHeader("Content-Length", "0");
  SetHeader("Location", location_->GetRedirectUrl());
  write_buffer_.AppendDataToBuffer(SerializeStatusAndHeader());
  return kComplete;
}

HttpResponse::CreateResponsePhase HttpResponse::MakeAutoIndexResponse(
    const std::string &abs, const std::string &relative) {
  Result<std::string> body_res = MakeAutoIndex(abs, relative);
  if (body_res.IsErr()) {
    return MakeErrorResponse(SERVER_ERROR);
  }

  SetStatus(OK, StatusCodes::GetMessage(OK));

  SetHeader("Content-Type", "text/html");

  return MakeResponse(body_res.Ok());
}

Result<std::string> HttpResponse::GetResponsableIndexPagePath() {
  const std::vector<std::string> &index_pages = location_->GetIndexPages();
  for (std::vector<std::string>::const_iterator it = index_pages.begin();
       it != index_pages.end(); ++it) {
    std::string abs_index_file_path =
        location_->GetAbsolutePath(location_->GetPathPattern() + *it);
    std::cout << "abs_index_file_path: " << abs_index_file_path << std::endl;
    if (utils::IsFileExist(abs_index_file_path) &&
        utils::IsReadableFile(abs_index_file_path)) {
      return abs_index_file_path;
    }
  }
  return Error();
}

HttpResponse::CreateResponsePhase HttpResponse::MakeErrorResponse(
    const HttpStatus status) {
  SetStatus(status, StatusCodes::GetMessage(status));

  headers_.clear();
  SetHeader("Connection", "close");

  if (location_ == NULL)
    return MakeResponse(SerializeErrorResponseBody(status));

  const std::map<http::HttpStatus, std::string> &error_pages =
      location_->GetErrorPages();
  if (error_pages.find(status) == error_pages.end() ||
      RegisterFile(error_pages.at(status)).IsErr()) {
    return MakeResponse(SerializeErrorResponseBody(status));
  } else {
    SetHeader("Content-Type",
              ContentTypes::GetContentTypeFromExt(
                  utils::GetExetension(error_pages.at(status))));
    return kStatusAndHeader;
  }
}

std::string HttpResponse::SerializeErrorResponseBody(HttpStatus status) {
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

bool HttpResponse::IsAllDataWritingCompleted() {
  return phase_ == kComplete && write_buffer_.empty();
}

bool HttpResponse::IsWriteBufferEmpty() const {
  return write_buffer_.empty();
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

bool HttpResponse::IsRequestHasConnectionClose(const HttpRequest &request) {
  Result<const http::HeaderMap::mapped_type &> header_res =
      request.GetHeader("Connection");
  if (header_res.IsErr())
    return false;
  const http::HeaderMap::mapped_type &header = header_res.Ok();

  return std::find(header.begin(), header.end(), "close") != header.end();
}

namespace {
std::string GetTimeStamp() {
  std::stringstream ss;
  struct timeval time_now;
  gettimeofday(&time_now, NULL);
  time_t msecs_time = (time_now.tv_sec * 1000000 + time_now.tv_usec);

  ss << msecs_time;
  return ss.str();
}

bool AppendBytesToFile(const std::string &path,
                       const utils::ByteVector &bytes) {
  bool file_exist = utils::IsFileExist(path);
  bool has_error = false;

  int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  has_error = fd < 0;

  if (has_error == false) {
    if (bytes.empty() == false) {
      ssize_t write_res =
          write(fd, bytes.SubstrBeforePos(bytes.size()).c_str(), bytes.size());
      has_error = write_res < static_cast<ssize_t>(bytes.size());
    }
    close(fd);
  }
  if (has_error && file_exist == false)
    remove(path.c_str());
  return has_error == false;
}
}  // namespace

}  // namespace http

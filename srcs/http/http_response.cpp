#include "http/http_response.hpp"

#include <algorithm>
#include <vector>

#include "http/http_constants.hpp"
#include "http/http_request.hpp"
#include "utils/io.hpp"
#include "utils/string.hpp"

namespace http {

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse &rhs) {
  *this = rhs;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &rhs) {
  if (this != &rhs) {
    status_ = rhs.status_;
    headers_ = rhs.headers_;
    status_line_ = rhs.status_line_;
    body_ = rhs.body_;
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::AppendHeader(const std::string &header,
                                const std::string &value) {
  headers_[header].push_back(value);
}

//========================================================================
// setter, getter

void HttpResponse::SetStatusLine(const std::string &status_line) {
  status_line_ = status_line;
}
void HttpResponse::SetBody(const std::string &body) {
  body_ = body;
}

const std::string &HttpResponse::GetStatusLine() const {
  return status_line_;
}
const std::string &HttpResponse::GetBody() const {
  return body_;
}

void HttpResponse::Write(int fd) const {
  WriteStatusLine(fd);
  WriteHeaders(fd);
  WriteBody(fd);
}

void HttpResponse::WriteStatusLine(int fd) const {
  utils::PutStrFd(status_line_, fd);
  utils::PutStrFd(http::kCrlf, fd);
}

void HttpResponse::WriteHeaders(int fd) const {
  for (HeaderMap::const_iterator headers_it = headers_.begin();
       headers_it != headers_.end(); ++headers_it) {
    typedef HeaderMap::mapped_type HeaderValuesType;
    HeaderValuesType header_values = headers_it->second;
    for (HeaderValuesType::const_iterator value_it = header_values.begin();
         value_it != header_values.end(); ++value_it) {
      const std::string header = headers_it->first + ": " + *value_it;
      utils::PutStrFd(header, fd);
      utils::PutStrFd(http::kCrlf, fd);
    }
  }
  utils::PutStrFd(http::kCrlf, fd);
}

void HttpResponse::WriteBody(int fd) const {
  utils::PutStrFd(body_, fd);
  utils::PutStrFd(http::kCrlf, fd);
}

//========================================================================
//
void HttpResponse::MakeResponse(const config::VirtualServerConf &vserver,
                                const HttpRequest &request) {
  // path から LocationConf を取得
  const config::LocationConf *location = vserver.GetLocation(request.GetPath());
  if (!location) {
    MakeErrorResponse(NULL, request, NOT_FOUND);
    return;
  }
  printf("===== Location =====\n");
  location->Print();

  if (location->GetIsCgi()) {
    MakeCgiReponse(*location, request);
  } else if (!location->GetRedirectUrl().empty()) {
    MakeRedirectResponse(*location, request);
  } else {
    MakeFileResponse(*location, request);
  }
}

bool HttpResponse::MakeErrorResponse(const config::LocationConf *location,
                                     const HttpRequest &request,
                                     HttpStatus status) {
  // TODO: 実装する
  (void)location;
  (void)request;
  (void)status;
  SetStatusLine("HTTP/1.1 404 Not Found");
  // TODO: location にエラーページが設定されていればそれをBodyにセットする
  return true;
}

bool HttpResponse::MakeFileResponse(const config::LocationConf &location,
                                    const HttpRequest &request) {
  std::string file_data;

  const std::string &abs_file_path = location.GetRootDir() + request.GetPath();
  if (!utils::IsFileExist(abs_file_path)) {
    MakeErrorResponse(&location, request, NOT_FOUND);
    return false;
  }

  if (utils::IsDir(abs_file_path)) {
    SetBody(MakeAutoIndex(location.GetRootDir(), request.GetPath()));
    SetStatusLine("HTTP/1.1 200 OK");
    AppendHeader("Content-Type", "text/html");
    return true;
  }

  if (!utils::ReadFile(abs_file_path, file_data)) {
    MakeErrorResponse(&location, request, FORBIDDEN);
    return false;
  }

  SetStatusLine("HTTP/1.1 200 OK");
  SetBody(file_data);
  AppendHeader("Content-Type", "text/plain");
  return true;
}

bool HttpResponse::MakeRedirectResponse(const config::LocationConf &location,
                                        const HttpRequest &request) {
  // TODO: 実装する
  (void)location;
  (void)request;
  return true;
}

bool HttpResponse::MakeCgiReponse(const config::LocationConf &location,
                                  const HttpRequest &request) {
  // TODO: 実装する
  (void)location;
  (void)request;
  return true;
}

}  // namespace http

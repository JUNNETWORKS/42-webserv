#include "http/http_response.hpp"

#include <unistd.h>

#include <algorithm>
#include <vector>

#include "cgi/cgi_request.hpp"
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

void HttpResponse::Clear() {
  status_ = OK;
  headers_.clear();
  status_line_.clear();
  body_.clear();
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

std::string read_child(int fd) {
  char buf[1024 + 1];

  std::string s;
  while (1) {
    ssize_t read_ret;
    // std::cout << "into read " << read_ret << std::endl;
    read_ret = read(fd, buf, 1024);
    // std::cout << "read_ret " << read_ret << std::endl;
    if (read_ret < 0) {
      std::cerr << "MakeCgiReponse read err" << std::endl;
      exit(1);
    }
    if (read_ret == 0) {
      break;
    }
    buf[read_ret] = '\0';
    s += std::string(buf);
  }
  // std::cout << "CGI" << std::endl;
  // std::cout << "--- --- ---" << std::endl;
  // std::cout << "body" << std::endl;
  // std::cout << body_ << std::endl;
  return s;
}

bool HttpResponse::MakeCgiReponse(const config::LocationConf &location,
                                  const HttpRequest &request) {
  // TODO: 実装する
  // (void)location;
  // (void)request;
  std::string req_path = request.GetPath();
  std::string abs_path =
      location.GetRootDir() + "/" +
      req_path.replace(0, location.GetPathPattern().length(), "");

  std::cout << "--- cgi ---" << std::endl;
  std::cout << "request.GetPath() : " << request.GetPath() << std::endl;
  std::cout << "abs_path          : " << request.GetPath() << std::endl;
  std::cout << "--- --- ---" << std::endl;

  cgi::CgiRequest cgi(abs_path, request, location);

  int parentsock = cgi.ForkAndExecuteCgi();

  std::string cgi_res = read_child(parentsock);
  close(parentsock);

  // とりあえず返せるように
  SetStatusLine("HTTP/1.1 200 OK");
  SetBody(cgi_res);
  AppendHeader("Content-Type", "text/plain");
  return true;
}

}  // namespace http

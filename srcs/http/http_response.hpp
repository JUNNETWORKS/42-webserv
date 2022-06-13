#ifndef HTTP_HTTP_RESPONSE_HPP_
#define HTTP_HTTP_RESPONSE_HPP_

#include <map>
#include <string>
#include <vector>

#include "config/virtual_server_conf.hpp"
#include "http/http_request.hpp"
#include "http/http_status.hpp"
#include "http/types.hpp"
#include "result/result.hpp"

namespace http {

using namespace result;

class HttpResponse {
 private:
  HttpStatus status_;
  HeaderMap headers_;
  std::string status_line_;
  std::string body_;

 public:
  HttpResponse();

  HttpResponse(const HttpResponse &rhs);

  HttpResponse &operator=(const HttpResponse &rhs);

  ~HttpResponse();

  void SetStatusLine(const std::string &status_line);
  void AppendHeader(const std::string &header, const std::string &value);
  void SetBody(const std::string &body);

  const std::string &GetStatusLine() const;
  const std::string &GetBody() const;

  void MakeResponse(const config::VirtualServerConf &vserver,
                    const HttpRequest &request);
  bool MakeErrorResponse(const config::LocationConf *location,
                         const HttpRequest &request, HttpStatus status);

  std::string MakeAutoIndex(const std::string &root_path,
                            const std::string &relative_path) const;

  void Write(int fd) const;

 private:
  void WriteStatusLine(int fd) const;
  void WriteHeaders(int fd) const;
  void WriteBody(int fd) const;

  // Making response
  bool MakeFileResponse(const config::LocationConf &location,
                        const HttpRequest &request);
  bool MakeRedirectResponse(const config::LocationConf &location,
                            const HttpRequest &request);
  bool MakeCgiReponse(const config::LocationConf &location,
                      const HttpRequest &request);
};

}  // namespace http

#endif

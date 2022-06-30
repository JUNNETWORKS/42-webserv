#ifndef HTTP_HTTP_FILE_RESPONSE_HPP_
#define HTTP_HTTP_FILE_RESPONSE_HPP_

#include "http/http_response.hpp"

namespace http {

class HttpFileResponse : public HttpResponse {
 public:
  HttpFileResponse(const config::LocationConf *location, server::Epoll *epoll);
  ~HttpFileResponse();

  virtual void MakeResponse(server::ConnSocket *conn_sock);

 private:
  HttpFileResponse();
  HttpFileResponse(const HttpFileResponse &rhs);
  HttpFileResponse &operator=(const HttpFileResponse &rhs);

  std::string MakeAutoIndex(const std::string &root_path,
                            const std::string &relative_path) const;
};

}  // namespace http

#endif

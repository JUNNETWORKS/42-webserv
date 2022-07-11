#ifndef HTTP_HTTP_CGI_RESPONSE_HPP_
#define HTTP_HTTP_CGI_RESPONSE_HPP_

#include "cgi/cgi_process.hpp"
#include "http/http_response.hpp"
#include "http/http_status.hpp"

namespace http {

class HttpCgiResponse : public HttpResponse {
 private:
  cgi::CgiProcess *cgi_process_;

 public:
  HttpCgiResponse(const config::LocationConf *location, server::Epoll *epoll);
  ~HttpCgiResponse();

 private:
  HttpCgiResponse::CreateResponsePhase LoadRequest(
      server::ConnSocket *conn_sock);
  Result<CreateResponsePhase> MakeResponseBody();

  CreateResponsePhase MakeDocumentResponse(server::ConnSocket *conn_sock);
  CreateResponsePhase MakeLocalRedirectResponse(server::ConnSocket *conn_sock);
  CreateResponsePhase MakeClientRedirectResponse(server::ConnSocket *conn_sock);

  void SetStatusFromCgiResponse();
  void SetHeadersFromCgiResponse();

  // LocalRedirect の結果に基づき新しいリクエストを作成
  HttpRequest CreateLocalRedirectRequest(const HttpRequest &request);
};

}  // namespace http

#endif

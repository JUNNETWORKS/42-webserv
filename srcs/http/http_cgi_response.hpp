#ifndef HTTP_HTTP_CGI_RESPONSE_HPP_
#define HTTP_HTTP_CGI_RESPONSE_HPP_

#include "cgi/cgi_process.hpp"
#include "http/http_response.hpp"
#include "http/http_status.hpp"

namespace http {

class HttpCgiResponse : public HttpResponse {
 private:
  cgi::CgiProcess *cgi_process_;

  enum CgiPhase { kSetupCgiTypeSpecificInfo, kWritingToInetSocket };
  CgiPhase cgi_phase_;

 public:
  HttpCgiResponse(const config::LocationConf *location, server::Epoll *epoll);
  virtual ~HttpCgiResponse();

  virtual void MakeResponse(server::ConnSocket *conn_sock);
  virtual void GrowResponse(server::ConnSocket *conn_sock);
  virtual Result<void> Write(int fd);

  // データ書き込みが可能か
  virtual bool IsReadyToWrite();

  // すべてのデータの write が完了したか
  virtual bool IsAllDataWritingCompleted();

 private:
  void MakeDocumentResponse(server::ConnSocket *conn_sock);
  void MakeLocalRedirectResponse(server::ConnSocket *conn_sock);
  void MakeClientRedirectResponse(server::ConnSocket *conn_sock);

  void SetStatusFromCgiResponse();
  void SetHeadersFromCgiResponse();

  // LocalRedirect の結果に基づき新しいリクエストを作成
  HttpRequest CreateLocalRedirectRequest(const HttpRequest &request);
};

}  // namespace http

#endif

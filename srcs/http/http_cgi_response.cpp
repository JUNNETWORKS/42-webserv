#include "http/http_cgi_response.hpp"

namespace http {

HttpCgiResponse::HttpCgiResponse(const config::LocationConf *location,
                                 server::Epoll *epoll)
    : HttpResponse(location, epoll), cgi_process_(location, epoll) {}

HttpCgiResponse::~HttpCgiResponse() {}

void HttpCgiResponse::MakeResponse(server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();

  if (!cgi_process_.IsCgiExecuted()) {
    if (cgi_process_.RunCgi(request).IsErr()) {
      MakeErrorResponse(request, SERVER_ERROR);
      return;
    }
  }
}

Result<void> HttpCgiResponse::Write(int fd);

// データ書き込みが可能か
bool HttpCgiResponse::IsReadyToWrite() {
  cgi::CgiResponse *cgi_response = cgi_process_.GetCgiResponse();
  return cgi_process_.IsCgiExecuted() &&
         cgi_response->GetResponseType() != cgi::CgiResponse::kNotIdentified &&
         !cgi_response->GetBody().empty();
}

// すべてのデータの write が完了したか
bool HttpCgiResponse::IsAllDataWritingCompleted();

}  // namespace http

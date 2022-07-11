#include "http/http_cgi_response.hpp"

#include <unistd.h>

namespace http {

HttpCgiResponse::HttpCgiResponse(const config::LocationConf *location,
                                 server::Epoll *epoll)
    : HttpResponse(location, epoll),
      cgi_process_(new cgi::CgiProcess(location, epoll)),
      cgi_phase_(kSetupCgiTypeSpecificInfo) {}

HttpCgiResponse::~HttpCgiResponse() {
  printf("HttpCgiResponse::~HttpCgiResponse\n");
  if (cgi_process_->IsRemovable()) {
    fprintf(stderr, "delete cgi_process_\n");
    delete cgi_process_;
  } else {
    epoll_->Unregister(cgi_process_->GetFde());
    cgi_process_->SetIsRemovable(true);
  }
}

void HttpCgiResponse::LoadRequest(server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();
  // TODO: 現在 cgi_request.RunCgi()
  // ではファイルの有無に関するエラーチェックをしていないので､
  // 存在しないCGIへのリクエストをするとInternalServerErrorが返ってくる｡
  if (cgi_process_->IsCgiExecuted() == false) {
    if (cgi_process_->RunCgi(request).IsErr()) {
      MakeErrorResponse(SERVER_ERROR);
    }
    return;
  }

  cgi::CgiResponse::ResponseType type =
      cgi_process_->GetCgiResponse()->GetResponseType();

  if (type == cgi::CgiResponse::kNotIdentified) {
    // CGIレスポンスタイプが決まっていないのに
    // CgiProcessが削除可能な状態(Unisockが0を返してきている)ならエラー
    if (cgi_process_->IsRemovable()) {
      MakeErrorResponse(SERVER_ERROR);
    }
    return;
  }

  if (type == cgi::CgiResponse::kParseError) {
    MakeErrorResponse(SERVER_ERROR);
    return;
    // TODO:CGIプロセスがまだ生きている可能性があるので､
    // CGIプロセスの出力をWrite()しないようにする必要がある｡
  } else if (type == cgi::CgiResponse::kLocalRedirect) {
    MakeLocalRedirectResponse(conn_sock);
    phase_ = kComplete;
    return;
  } else if (type == cgi::CgiResponse::kDocumentResponse) {
    MakeDocumentResponse(conn_sock);
  } else if (type == cgi::CgiResponse::kClientRedirect ||
             type == cgi::CgiResponse::kClientRedirectWithDocument) {
    MakeClientRedirectResponse(conn_sock);
  }
  phase_ = kStatusAndHeader;
}

Result<HttpCgiResponse::CreateResponsePhase>
HttpCgiResponse::PrepareResponseBody() {
  if (file_fd_ >= 0) {
    Result<bool> file_res = ReadFile();
    if (file_res.IsErr()) {
      return file_res.Err();
    } else {
      return file_res.Ok() ? kComplete : kBody;
    }
  } else {
    // TODO: chunked-encoding の設定
    utils::ByteVector &cgi_response_body =
        cgi_process_->GetCgiResponse()->GetBody();
    write_buffer_.AppendDataToBuffer(cgi_response_body);
    cgi_response_body.clear();

    return cgi_process_->IsRemovable() ? kComplete : kBody;
  }
}

void HttpCgiResponse::MakeDocumentResponse(server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();

  SetStatusFromCgiResponse();
  SetHeadersFromCgiResponse();

  if (IsRequestHasConnectionClose(request)) {
    SetHeader("Connection", "close");
  }
}

void HttpCgiResponse::MakeLocalRedirectResponse(server::ConnSocket *conn_sock) {
  // LocalRedirect を反映させた Request を2番目にinsertする
  std::deque<http::HttpRequest> &requests = conn_sock->GetRequests();
  HttpRequest new_request = CreateLocalRedirectRequest(requests.front());
  requests.insert(requests.begin() + 1, new_request);
}

void HttpCgiResponse::MakeClientRedirectResponse(
    server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

  if (cgi_response->GetHeader("Status").IsOk()) {
    SetStatusFromCgiResponse();
  } else {
    SetStatus(FOUND);
  }
  SetHeadersFromCgiResponse();

  if (IsRequestHasConnectionClose(request)) {
    SetHeader("Connection", "close");
  }
}

void HttpCgiResponse::SetStatusFromCgiResponse() {
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

  Result<std::string> status_result = cgi_response->GetHeader("Status");
  if (status_result.IsErr()) {
    // TODO: エラーを返すべき?
    return;
  }

  std::string status_value = status_result.Ok();
  std::size_t space_pos = status_value.find(" ");
  if (space_pos == std::string::npos) {
    return;
  }
  std::string status_str = status_value.substr(0, space_pos);
  Result<unsigned long> status = utils::Stoul(status_str);
  std::string status_msg = status_value.substr(space_pos);
  if (status.IsErr() || !StatusCodes::IsHttpStatus(status.Ok()) ||
      status_msg.empty()) {
    return;
  }
  SetStatus(static_cast<HttpStatus>(status.Ok()), status_msg);
}

void HttpCgiResponse::SetHeadersFromCgiResponse() {
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();
  const cgi::CgiResponse::HeaderVecType &header_vec =
      cgi_response->GetHeaders();
  for (cgi::CgiResponse::HeaderVecType::const_iterator it = header_vec.begin();
       it != header_vec.end(); ++it) {
    if (it->first != "STATUS") {
      SetHeader(it->first, it->second);
    }
  }
}

HttpRequest HttpCgiResponse::CreateLocalRedirectRequest(
    const HttpRequest &request) {
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

  Result<std::string> location = cgi_response->GetHeader("LOCATION");

  HttpRequest new_request(request);
  new_request.SetPath(location.Ok());
  return new_request;
}

}  // namespace http

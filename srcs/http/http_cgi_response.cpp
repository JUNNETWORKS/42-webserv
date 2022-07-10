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

Result<void> HttpCgiResponse::Write(int fd) {
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

  // TODO
  //  Result<ssize_t> status_header_res = WriteStatusAndHeader(fd);
  //  if (status_header_res.IsErr()) {
  //    return status_header_res.Err();
  //  }
  //  if (status_header_res.Ok() > 0) {
  //    // WriteStatusAndHeader()
  //    // で書き込みが行われた後にノンブロッキングで書き込める保証はない
  //    return Result<void>();
  //  }

  // エラーのレスポンスを返す
  // if (!body_bytes_.empty()) {
  //   Result<ssize_t> result = WriteBody(fd);
  //   if (result.IsErr()) {
  //     return result.Err();
  //   }
  // }

  // TODO: chunked-encoding の設定
  utils::ByteVector &response_body = cgi_response->GetBody();
  if (!response_body.empty()) {
    ssize_t write_res = write(fd, response_body.data(), response_body.size());
    if (write_res < 0) {
      printf("HttpCgiReponse::Write write() return %ld", write_res);
      return Error();
    }
    response_body.EraseHead(write_res);
  }
  return Result<void>();
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
    // TODO:CGIプロセスがまだ生きている可能性があるので､
    // CGIプロセスの出力をWrite()しないようにする必要がある｡
  } else if (type == cgi::CgiResponse::kDocumentResponse) {
    MakeDocumentResponse(conn_sock);
  } else if (type == cgi::CgiResponse::kLocalRedirect) {
    MakeLocalRedirectResponse(conn_sock);
  } else if (type == cgi::CgiResponse::kClientRedirect ||
             type == cgi::CgiResponse::kClientRedirectWithDocument) {
    MakeClientRedirectResponse(conn_sock);
  }
}

Result<void> HttpCgiResponse::PrepareToWrite(server::ConnSocket *conn_sock) {
  return Result<void>();
}

// // データ書き込みが可能か
// bool HttpCgiResponse::IsReadyToWrite() {
//   cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

//   // まだヘッダーやStatusなどの情報が確定していない
//   // LocalRedirectの場合はレスポンスを書き込まないので常にFalse
//   if (cgi_phase_ == kSetupCgiTypeSpecificInfo ||
//       cgi_response->GetResponseType() == cgi::CgiResponse::kLocalRedirect) {
//     return false;
//   }

//   return phase_ == kStatusAndHeader ||
//          (phase_ == kBody && cgi_process_->IsCgiExecuted() &&
//           (!cgi_response->GetBody().empty() || IsReadyToWriteBody() ||
//            IsReadyToWriteFile()));
// }

// すべてのデータの write が完了したか
bool HttpCgiResponse::IsAllDataWritingCompleted() {
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

  // まだヘッダーやStatusなどの情報が確定していない場合はFalse
  if (cgi_phase_ == kSetupCgiTypeSpecificInfo) {
    return false;
  }

  // LocalRedirectの場合はレスポンスを書き込まないので常にTrue
  if (cgi_response->GetResponseType() == cgi::CgiResponse::kLocalRedirect) {
    return true;
  }

  // Trueの場合
  // - CgiProcessが終了済み && バッファの全てのデータが書き込み完了
  return HttpResponse::IsAllDataWritingCompleted() &&
         cgi_process_->IsRemovable() &&
         cgi_process_->GetCgiResponse()->GetBody().empty();
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

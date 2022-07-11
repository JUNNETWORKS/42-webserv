#include "http/http_cgi_response.hpp"

#include <unistd.h>

namespace http {

const std::string HttpCgiResponse::kLastChunk = "0" + kCrlf + kCrlf;

HttpCgiResponse::HttpCgiResponse(const config::LocationConf *location,
                                 server::Epoll *epoll)
    : HttpResponse(location, epoll),
      cgi_process_(new cgi::CgiProcess(location, epoll)) {}

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

HttpCgiResponse::CreateResponsePhase HttpCgiResponse::LoadRequest(
    server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();
  // TODO: 現在 cgi_request.RunCgi()
  // ではファイルの有無に関するエラーチェックをしていないので､
  // 存在しないCGIへのリクエストをするとInternalServerErrorが返ってくる｡
  if (cgi_process_->IsCgiExecuted() == false) {
    if (cgi_process_->RunCgi(request).IsErr()) {
      return MakeErrorResponse(SERVER_ERROR);
    }
    return phase_;
  }

  cgi::CgiResponse::ResponseType type =
      cgi_process_->GetCgiResponse()->GetResponseType();

  switch (type) {
    case cgi::CgiResponse::kParseError:
      return MakeErrorResponse(SERVER_ERROR);

    case cgi::CgiResponse::kLocalRedirect:
      return MakeLocalRedirectResponse(conn_sock);

    case cgi::CgiResponse::kDocumentResponse:
      return MakeDocumentResponse(conn_sock);

    case cgi::CgiResponse::kClientRedirect:
    case cgi::CgiResponse::kClientRedirectWithDocument:
      return MakeClientRedirectResponse(conn_sock);

    default:  // kNotIdentified
      // CGIレスポンスタイプが決まっていないのに
      // CgiProcessが削除可能な状態(Unisockが0を返してきている)ならエラー
      if (cgi_process_->IsRemovable()) {
        return MakeErrorResponse(SERVER_ERROR);
      }
      return phase_;
  }
}

Result<HttpCgiResponse::CreateResponsePhase>
HttpCgiResponse::MakeResponseBody() {
  if (file_fd_ >= 0) {
    Result<bool> file_res = ReadFile();
    if (file_res.IsErr()) {
      return file_res.Err();
    } else {
      return file_res.Ok() ? kComplete : kBody;
    }
  } else {
    utils::ByteVector &cgi_response_body =
        cgi_process_->GetCgiResponse()->GetBody();
    write_buffer_.AppendDataToBuffer(ConvertChunkResponse(cgi_response_body));
    cgi_response_body.clear();

    if (cgi_process_->IsRemovable()) {
      write_buffer_.AppendDataToBuffer(kLastChunk);
      return kComplete;
    } else {
      return kBody;
    }
  }
}

std::string HttpCgiResponse::ConvertChunkResponse(utils::ByteVector data) {
  std::stringstream ss;
  while (data.empty() == false) {
    size_t chunk_size =
        data.size() < kMaxChunkSize ? data.size() : kMaxChunkSize;
    ss << std::hex << chunk_size;
    ss << kCrlf;
    ss << data.SubstrBeforePos(chunk_size);
    ss << kCrlf;
    data.erase(data.begin(), data.begin() + chunk_size);
  }
  return ss.str();
}

HttpCgiResponse::CreateResponsePhase HttpCgiResponse::MakeDocumentResponse(
    server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();

  SetStatusFromCgiResponse();
  SetHeadersFromCgiResponse();

  if (IsRequestHasConnectionClose(request)) {
    SetHeader("Connection", "close");
  }
  return kStatusAndHeader;
}

HttpCgiResponse::CreateResponsePhase HttpCgiResponse::MakeLocalRedirectResponse(
    server::ConnSocket *conn_sock) {
  // LocalRedirect を反映させた Request を2番目にinsertする
  std::deque<http::HttpRequest> &requests = conn_sock->GetRequests();
  HttpRequest new_request = CreateLocalRedirectRequest(requests.front());
  if (new_request.GetLocalRedirectCount() > 10) {
    return MakeErrorResponse(SERVER_ERROR);
  } else {
    requests.insert(requests.begin() + 1, new_request);
    return kComplete;
  }
}

HttpCgiResponse::CreateResponsePhase
HttpCgiResponse::MakeClientRedirectResponse(server::ConnSocket *conn_sock) {
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
  return kStatusAndHeader;
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
  new_request.SetLocalRedirectCount(request.GetLocalRedirectCount() + 1);
  return new_request;
}

}  // namespace http

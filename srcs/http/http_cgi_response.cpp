#include "http/http_cgi_response.hpp"

#include <unistd.h>

namespace http {

const std::string HttpCgiResponse::kLastChunk = "0" + kCrlf + kCrlf;

HttpCgiResponse::HttpCgiResponse(const config::LocationConf *location,
                                 server::Epoll *epoll,
                                 server::ConnSocket *socket)
    : HttpResponse(location, epoll, kHttpCgiResponse),
      cgi_process_(new cgi::CgiProcess(location, epoll, socket)) {}

HttpCgiResponse::~HttpCgiResponse() {
  printf("HttpCgiResponse::~HttpCgiResponse\n");
  if (cgi_process_->IsRemovable()) {
    epoll_->Unregister(cgi_process_->GetFde());
    delete cgi_process_;
  } else {
    cgi_process_->SetIsRemovable(true);
  }
}

HttpCgiResponse::CreateResponsePhase HttpCgiResponse::ExecuteRequest(
    server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();
  if (cgi_process_->IsCgiExecuted() == false) {
    const http::HttpStatus cgi_res_code =
        cgi_process_->RunCgi(conn_sock, request);
    if (cgi_res_code != http::OK) {
      return MakeErrorResponse(cgi_res_code);
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
    write_buffer_.AppendDataToBuffer(ConvertToChunkResponse(cgi_response_body));
    cgi_response_body.clear();

    if (cgi_process_->IsRemovable()) {
      write_buffer_.AppendDataToBuffer(kLastChunk);
      return kComplete;
    } else {
      return kBody;
    }
  }
}

std::string HttpCgiResponse::ConvertToChunkResponse(utils::ByteVector data) {
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
  SetHeader("Transfer-Encoding", "chunked");
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
    if (SetStatusFromCgiResponse().IsErr()) {
      return MakeErrorResponse(SERVER_ERROR);
    }
  } else {
    SetStatus(FOUND);
  }
  SetHeadersFromCgiResponse();

  if (IsRequestHasConnectionClose(request)) {
    SetHeader("Connection", "close");
  }
  SetHeader("Transfer-Encoding", "chunked");
  return kStatusAndHeader;
}

Result<void> HttpCgiResponse::SetStatusFromCgiResponse() {
  cgi::CgiResponse *cgi_response = cgi_process_->GetCgiResponse();

  Result<std::string> status_result = cgi_response->GetHeader("Status");
  if (status_result.IsErr()) {
    return Error();
  }

  std::string status_with_msg = status_result.Ok();
  utils::TrimString(status_with_msg, " ");

  // Status と Message を分ける
  std::size_t space_pos = status_with_msg.find(" ");
  if (space_pos == std::string::npos) {
    return Error();
  }
  std::size_t msg_pos = space_pos;
  while (msg_pos < status_with_msg.length() &&
         status_with_msg[msg_pos] == ' ') {
    msg_pos++;
  }
  if (msg_pos == status_with_msg.length()) {
    return Error();
  }

  std::string status_str = status_with_msg.substr(0, space_pos);
  Result<unsigned long> status = utils::Stoul(status_str);
  std::string status_msg = status_with_msg.substr(msg_pos);
  if (status.IsErr() || !StatusCodes::IsHttpStatus(status.Ok()) ||
      status_msg.empty()) {
    return Error();
  }
  SetStatus(static_cast<HttpStatus>(status.Ok()), status_msg);
  return Result<void>();
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

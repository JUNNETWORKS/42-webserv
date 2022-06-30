#include "http/http_file_response.hpp"

#include "utils/io.hpp"

namespace http {

HttpFileResponse::HttpFileResponse(const config::LocationConf *location,
                                   server::Epoll *epoll)
    : HttpResponse(location, epoll) {}

HttpFileResponse::~HttpFileResponse() {}

void HttpFileResponse::MakeResponse(server::ConnSocket *conn_sock) {
  http::HttpRequest &request = conn_sock->GetRequests().front();

  const std::string &abs_file_path =
      location_->GetRootDir() + request.GetPath();
  if (!utils::IsFileExist(abs_file_path)) {
    MakeErrorResponse(request, NOT_FOUND);
    return;
  }

  if (utils::IsDir(abs_file_path)) {
    body_bytes_ = MakeAutoIndex(location_->GetRootDir(), request.GetPath());
    SetStatus(OK, StatusCodes::GetMessage(OK));
    AppendHeader("Content-Type", "text/html");
    return;
  }

  if (utils::IsReadableFile(abs_file_path)) {
    MakeErrorResponse(request, FORBIDDEN);
    return;
  }

  SetStatus(OK, StatusCodes::GetMessage(OK));
  AppendHeader("Content-Type", "text/plain");
  RegisterFile(abs_file_path);
}

}  // namespace http

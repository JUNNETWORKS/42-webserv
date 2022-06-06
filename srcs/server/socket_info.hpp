#ifndef SERVER_SOCKET_INFO_HPP_
#define SERVER_SOCKET_INFO_HPP_

#include <vector>

#include "http/http_request.hpp"
#include "http/http_response.hpp"
#include "utils/ByteVector.hpp"

namespace server {

struct SocketInfo {
  enum ESockType { ListenSock, ConnSock };
  enum EPhase { Request, Response };

  int fd;
  std::string port;
  ESockType socktype;
  EPhase phase;  // リクエストが読み込み終わってないときは Request,
                 // 読み込み終わったら Response
  std::vector<http::HttpRequest> requests;
  http::HttpResponse response;
  utils::ByteVector buffer_;
};

}  // namespace server

#endif

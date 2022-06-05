#ifndef SERVER_EVENT_LOOP_HPP_
#define SERVER_EVENT_LOOP_HPP_

#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "config/config.hpp"
#include "http/http_request.hpp"
#include "http/http_response.hpp"
#include "server/socket_manager.hpp"
#include "utils/ByteVector.hpp"

namespace server {

struct SocketInfo {
  enum ESockType { ListenSock, ConnSock };
  enum EPhase { Request, Response };

  int fd;
  ESockType socktype;
  EPhase phase;  // リクエストが読み込み終わってないときは Request,
                 // 読み込み終わったら Response
  std::vector<http::HttpRequest> requests;
  http::HttpResponse response;
  utils::ByteVector buffer_;
};

int StartEventLoop(SocketManager &socket_manager, const config::Config &config);

}  // namespace server

#endif

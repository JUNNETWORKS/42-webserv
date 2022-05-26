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

#include "http/http_request.hpp"
#include "http/http_response.hpp"
#include "utils/ByteVector.hpp"

namespace server {

struct SocketInfo {
  enum ESockType { ListenSock, ConnSock };
  enum EPhase { Request, Response };

  int fd;
  ESockType socktype;
  EPhase phase;  // リクエストが読み込み終わってないときは Request,
                 // 読み込み終わったら Response
  http::HttpRequest request;
  http::HttpResponse response;
  utils::ByteVector buffer_;
};

int StartEventLoop(int listen_fd);

};  // namespace server

#endif

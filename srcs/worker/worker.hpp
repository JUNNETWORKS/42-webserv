#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "http/http.hpp"

namespace worker {

struct SocketInfo {
  enum ESockType { ListenSock, ConnSock };
  enum EPhase { Request, Response };

  int fd;
  ESockType socktype;
  EPhase phase;  // リクエストが読み込み終わってないときは Request,
                 // 読み込み終わったら Response
  // HttpRequest request;
  // HtppResponse response;
};

int startWorker(int listen_fd);

};  // namespace worker

#endif

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
#include "server/epoll.hpp"
#include "utils/ByteVector.hpp"

namespace server {

int StartEventLoop(Epoll &epoll);

}  // namespace server

#endif

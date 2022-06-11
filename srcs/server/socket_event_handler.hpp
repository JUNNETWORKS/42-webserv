#ifndef SERVER_SOCKET_EVENT_HANDLER_HPP_
#define SERVER_SOCKET_EVENT_HANDLER_HPP_

#include "epoll.hpp"
#include "result/result.hpp"

namespace server {

using namespace result;

// ConnectionSocketのイベントハンドラー
void HandleConnSocketEvent(int fd, unsigned int events, void *userdata,
                           Epoll *epoll);

// ListenSocketのイベントハンドラー
void HandleListenSocketEvent(int fd, unsigned int events, void *userdata,
                             Epoll *epoll);

}  // namespace server

#endif

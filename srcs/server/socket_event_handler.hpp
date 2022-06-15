#ifndef SERVER_SOCKET_EVENT_HANDLER_HPP_
#define SERVER_SOCKET_EVENT_HANDLER_HPP_

#include "epoll.hpp"
#include "result/result.hpp"

namespace server {

using namespace result;

// ConnectionSocketのイベントハンドラー
void HandleConnSocketEvent(FdEvent *fde, unsigned int events, void *data,
                           Epoll *epoll);

// ListenSocketのイベントハンドラー
void HandleListenSocketEvent(FdEvent *fde, unsigned int events, void *data,
                             Epoll *epoll);

}  // namespace server

#endif

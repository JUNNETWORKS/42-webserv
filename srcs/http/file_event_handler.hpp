#ifndef HTTP_FILE_EVENT_HANDLER_HPP_
#define HTTP_FILE_EVENT_HANDLER_HPP_

#include "server/epoll.hpp"

namespace http {

using namespace server;

void HandleFileEvent(FdEvent *fde, unsigned int events, void *data,
                     Epoll *epoll);

}  // namespace http

#endif

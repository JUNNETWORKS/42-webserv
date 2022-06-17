#ifndef CGI_CGI_EVENT_HANDLER_HPP_
#define CGI_CGI_EVENT_HANDLER_HPP_

#include "server/epoll.hpp"

namespace cgi {

void HandleCgiUniSockEvent(server::FdEvent *fde, unsigned int events,
                           void *data, server::Epoll *epoll);

}  // namespace cgi

#endif

#include "server/fd_event.hpp"

namespace server {

FdEvent::FdEvent(int fd, EFdType fd_type, void *ptr) {
  this->fd = fd;
  this->fd_type = fd_type;
  this->ptr = ptr;
}

}  // namespace server

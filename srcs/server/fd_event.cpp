#include "server/fd_event.hpp"

#include <cstdlib>

namespace server {

FdEvent::FdEvent() {
  this->fd = -1;
  this->fd_type = SocketFd;
  this->ptr = NULL;
  this->events = 0;
}

FdEvent::FdEvent(int fd, EFdType fd_type, void *ptr) {
  this->fd = fd;
  this->fd_type = fd_type;
  this->ptr = ptr;
  this->events = 0;
}

FdEvent::FdEvent(const FdEvent &rhs) {
  *this = rhs;
}

const FdEvent &FdEvent::operator=(const FdEvent &rhs) {
  if (this != &rhs) {
    this->fd = rhs.fd;
    this->fd_type = rhs.fd_type;
    this->ptr = rhs.ptr;
    this->events = rhs.events;
  }
  return *this;
}

FdEvent::~FdEvent() {}

}  // namespace server

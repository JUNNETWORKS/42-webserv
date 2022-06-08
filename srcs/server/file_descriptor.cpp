#include "server/fd.hpp"

namespace server {

FileDescriptor::FileDescriptor(int fd, EFdType fd_type, void *ptr) {
  this->fd = fd;
  this->fd_type = fd_type;
  this->ptr = ptr;
}

}  // namespace server

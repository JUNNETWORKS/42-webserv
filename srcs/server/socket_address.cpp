#include "server/socket_address.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>

namespace server {

SocketAddress::SocketAddress() : sockaddr_(NULL), sockaddr_len_(0) {}

SocketAddress::SocketAddress(const struct sockaddr *sockaddr,
                             const socklen_t sockaddr_len)
    : sockaddr_(NULL), sockaddr_len_(sockaddr_len) {
  sockaddr_ = (struct sockaddr *)new char[sockaddr_len];
  memcpy(sockaddr_, sockaddr, sockaddr_len);
}

SocketAddress::SocketAddress(const SocketAddress &rhs) {
  *this = rhs;
}

SocketAddress &SocketAddress::operator=(const SocketAddress &rhs) {
  if (this != &rhs) {
    delete[] sockaddr_;
    sockaddr_len_ = rhs.sockaddr_len_;
    sockaddr_ = (struct sockaddr *)new char[sockaddr_len_];
    memcpy(sockaddr_, rhs.sockaddr_, sockaddr_len_);
  }
  return *this;
}

SocketAddress::~SocketAddress() {
  delete[] sockaddr_;
}

void SocketAddress::SetSockaddr(const struct sockaddr *sockaddr,
                                const socklen_t sockaddr_len) {
  if (sockaddr_ != NULL) {
    delete[] sockaddr_;
  }
  sockaddr_ = (struct sockaddr *)new char[sockaddr_len];
  sockaddr_len_ = sockaddr_len;
  memcpy(sockaddr_, sockaddr, sockaddr_len);
}

const struct sockaddr *SocketAddress::GetSockaddr() const {
  return sockaddr_;
}

socklen_t SocketAddress::GetSockaddrLen() const {
  return sockaddr_len_;
}

std::string SocketAddress::GetIp() const {
  char host[NI_MAXHOST];
  if (getnameinfo(sockaddr_, sockaddr_len_, host, NI_MAXHOST, NULL, 0,
                  NI_NUMERICHOST) != 0) {
    return "";
  }
  return host;
}

std::string SocketAddress::GetName() const {
  char host[NI_MAXHOST];
  if (getnameinfo(sockaddr_, sockaddr_len_, host, NI_MAXHOST, NULL, 0,
                  NI_NAMEREQD) != 0) {
    return "";
  }
  return host;
}

std::string SocketAddress::GetPort() const {
  char service[NI_MAXSERV];
  if (getnameinfo(sockaddr_, sockaddr_len_, NULL, 0, service, NI_MAXSERV,
                  NI_NUMERICSERV) != 0) {
    return "";
  }
  return service;
}

}  // namespace server

#include "server/socket.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <deque>

#include "utils/inet_sockets.hpp"

namespace server {

// ========================================================================
// Socket

Socket::Socket(int fd, ESockType socktype, const std::string &port,
               const config::Config &config)
    : fd_(fd), socktype_(socktype), port_(port), config_(config) {}

Socket::Socket(const Socket &rhs)
    : fd_(rhs.fd_),
      socktype_(rhs.socktype_),
      port_(rhs.port_),
      config_(rhs.config_) {}

Socket::~Socket() {
  close(fd_);
}

int Socket::GetFd() const {
  return fd_;
}

const std::string &Socket::GetPort() const {
  return port_;
}

const config::Config &Socket::GetConfig() const {
  return config_;
}

Socket::ESockType Socket::GetSockType() const {
  return socktype_;
}

// ========================================================================
// ConnSocket

ConnSocket::ConnSocket(int fd, const std::string &port,
                       const config::Config &config)
    : Socket(fd, ListenSock, port, config), response_(NULL) {}

ConnSocket::~ConnSocket() {
  if (response_ != NULL) {
    delete response_;
  }
}

std::deque<http::HttpRequest> &ConnSocket::GetRequests() {
  return requests_;
}

bool ConnSocket::HasParsedRequest() {
  return !requests_.empty() && requests_.front().IsResponsible();
}

http::HttpResponse *ConnSocket::GetResponse() {
  return response_;
}

void ConnSocket::SetResponse(http::HttpResponse *response) {
  response_ = response;
}

utils::ByteVector &ConnSocket::GetBuffer() {
  return buffer_;
}

// ========================================================================
// ListenSocket

ListenSocket::ListenSocket(int fd, const std::string &port,
                           const config::Config &config)
    : Socket(fd, ListenSock, port, config) {}

Result<ConnSocket *> ListenSocket::AcceptNewConnection() {
  assert(socktype_ == ListenSock);

  struct sockaddr_storage client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  int conn_fd = accept4(fd_, (struct sockaddr *)&client_addr, &addrlen,
                        SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (conn_fd < 0) {
    return Error("accept");
  }

  utils::LogConnectionInfoToStdout(client_addr);

  ConnSocket *conn_sock = new ConnSocket(conn_fd, port_, config_);
  return conn_sock;
}

}  // namespace server

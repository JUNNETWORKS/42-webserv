#include "server/socket.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cassert>

#include "utils/inet_sockets.hpp"

namespace server {

Socket::Socket(int fd, ESockType socktype, const std::string &port,
               const config::Config &config)
    : fd_(fd), socktype_(socktype), port_(port), config_(config) {}

Socket::Socket(const Socket &rhs)
    : fd_(rhs.fd_),
      socktype_(rhs.socktype_),
      port_(rhs.port_),
      config_(rhs.config_),
      requests_(rhs.requests_),
      response_(rhs.response_),
      buffer_(rhs.buffer_) {}

Socket::~Socket() {
  close(fd_);
}

Result<Socket *> Socket::AcceptNewConnection() {
  assert(socktype_ == ListenSock);

  struct sockaddr_storage client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  int conn_fd = accept(fd_, (struct sockaddr *)&client_addr, &addrlen);
  if (conn_fd) {
    return Error("accept");
  }
  if (fcntl(conn_fd, F_SETFD, O_NONBLOCK) < 0) {
    return Error("fcntl");
  }

  utils::LogConnectionInfoToStdout(client_addr);

  Socket *conn_sock = new Socket(conn_fd, ConnSock, port_, config_);
  return conn_sock;
}

// ========================================================================
// Getter and Setter

int Socket::GetFd() const {
  return fd_;
}

const std::string &Socket::GetPort() const {
  return port_;
}

const config::Config &Socket::GetConfig() const {
  return config_;
}

std::vector<http::HttpRequest> &Socket::GetRequests() {
  return requests_;
}

http::HttpResponse &Socket::GetResponse() {
  return response_;
}

Socket::ESockType Socket::GetSockType() const {
  return socktype_;
}

utils::ByteVector &Socket::GetBuffer() {
  return buffer_;
}

}  // namespace server

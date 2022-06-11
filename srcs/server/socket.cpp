#include "server/socket.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cassert>

#include "utils/inet_sockets.hpp"

namespace server {

std::map<int, int> Socket::fd_reference_counter_;

Socket::Socket(int fd, ESockType socktype, const std::string &port)
    : fd_(fd), socktype_(socktype), port_(port) {
  IncreaseFdReferenceCount(fd_);
}

Socket::Socket(const Socket &rhs) {
  *this = rhs;
}

Socket &Socket::operator=(const Socket &rhs) {
  if (this != &rhs) {
    DecreaseFdReferenceCount(fd_);

    fd_ = rhs.fd_;
    socktype_ = rhs.socktype_;
    port_ = rhs.port_;
    requests_ = rhs.requests_;
    requests_ = rhs.requests_;

    IncreaseFdReferenceCount(fd_);
  }
  return *this;
}

Socket::~Socket() {
  DecreaseFdReferenceCount(fd_);
  if (GetFdReferenceCount(fd_) == 0) {
    close(fd_);
  }
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

  Socket *conn_sock = new Socket(conn_fd, ConnSock, port_);
  return conn_sock;
}

// ========================================================================
// Getter and Setter

int Socket::GetFd() const {
  return fd_;
}

void Socket::SetPort(const std::string &port) {
  port_ = port;
}

const std::string &Socket::GetPort() const {
  return port_;
}

Socket::ESockType Socket::GetSockType() const {
  return socktype_;
}

utils::ByteVector &Socket::GetBuffer() {
  return buffer_;
}

void Socket::IncreaseFdReferenceCount(int fd) {
  fd_reference_counter_[fd]++;
}

void Socket::DecreaseFdReferenceCount(int fd) {
  assert(fd_reference_counter_[fd] > 0);
  fd_reference_counter_[fd]--;
}

int Socket::GetFdReferenceCount(int fd) {
  return fd_reference_counter_[fd];
}

}  // namespace server

#include "server/socket.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <deque>

#include "utils/inet_sockets.hpp"

namespace server {

// ========================================================================
// Socket

Socket::Socket(int fd, const SocketAddress &server_addr,
               const config::Config &config)
    : fd_(fd), server_addr_(server_addr), config_(config) {}

Socket::Socket(const Socket &rhs)
    : fd_(rhs.fd_), server_addr_(rhs.server_addr_), config_(rhs.config_) {}

Socket::~Socket() {
  close(fd_);
}

int Socket::GetFd() const {
  return fd_;
}

std::string Socket::GetServerIp() const {
  return server_addr_.GetIp();
}

std::string Socket::GetServerPort() const {
  return server_addr_.GetPort();
}

const config::Config &Socket::GetConfig() const {
  return config_;
}

// ========================================================================
// ConnSocket

ConnSocket::ConnSocket(int fd, const SocketAddress &server_addr,
                       const SocketAddress &client_addr,
                       const config::Config &config)
    : Socket(fd, server_addr, config),
      client_addr_(client_addr),
      response_(NULL) {}

ConnSocket::~ConnSocket() {
  if (response_ != NULL) {
    delete response_;
  }
}

std::deque<http::HttpRequest> &ConnSocket::GetRequests() {
  return requests_;
}

std::string ConnSocket::GetRemoteIp() const {
  return client_addr_.GetIp();
}

std::string ConnSocket::GetRemoteName() const {
  return client_addr_.GetName();
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

ListenSocket::ListenSocket(int fd, const SocketAddress &server_addr,
                           const config::Config &config)
    : Socket(fd, server_addr, config) {}

Result<ConnSocket *> ListenSocket::AcceptNewConnection() {
  struct sockaddr_storage client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  int conn_fd = accept(fd_, (struct sockaddr *)&client_addr, &addrlen);
  if (conn_fd < 0) {
    return Error("accept");
  }
  int flags;
  if ((flags = fcntl(conn_fd, F_GETFL, 0)) < 0 ||
      fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    return Error("fcntl");
  }

  utils::LogConnectionInfoToStdout(client_addr);

  ConnSocket *conn_sock = new ConnSocket(
      conn_fd, server_addr_,
      SocketAddress((const struct sockaddr *)&client_addr, addrlen), config_);
  return conn_sock;
}

}  // namespace server

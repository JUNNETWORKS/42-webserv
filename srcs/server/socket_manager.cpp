#include "socket_manager.hpp"

#include <sys/epoll.h>
#include <unistd.h>

#include "server/event_loop.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {

SocketManager::SocketManager() {
  epfd_ = epoll_create(1);
}

SocketManager::SocketManager(const SocketManager &rhs) {
  *this = rhs;
}

SocketManager &SocketManager::operator=(const SocketManager &rhs) {
  if (this != &rhs) {
    epfd_ = rhs.epfd_;
  }
  return *this;
}

SocketManager::~SocketManager() {
  close(epfd_);
}

bool SocketManager::AppendListenFd(int fd) {
  return AppendNewSockFdIntoEpfd(fd, SocketInfo::ListenSock, EPOLLIN);
}

bool SocketManager::AppendListenFd(const std::vector<int> &fd_vec) {
  for (std::vector<int>::const_iterator it = fd_vec.begin(); it != fd_vec.end();
       ++it) {
    if (!AppendListenFd(*it)) {
      return false;
    }
  }
  return true;
}

bool SocketManager::AcceptNewConnection(int listen_fd) {
  struct sockaddr_storage client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
  fcntl(conn_fd, F_SETFD, O_NONBLOCK);
  if (!AppendNewSockFdIntoEpfd(conn_fd, SocketInfo::ConnSock,
                               EPOLLIN | EPOLLOUT)) {
    return false;
  }
  utils::LogConnectionInfoToStdout(client_addr);
  return true;
}

bool SocketManager::CloseConnFd(int fd) {
  close(fd);
  return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == 0;
}

struct epoll_event SocketManager::WaitEvent() {
  struct epoll_event epevarr[1];
  while (epoll_wait(epfd_, epevarr, 1, -1) == -1) {
    if (errno == EINTR)
      continue;  // wait epoll again if interrupted by signal
    else
      exit(EXIT_FAILURE);
  }
  return epevarr[0];
}

int SocketManager::GetEpollFd() const {
  return epfd_;
}

bool SocketManager::AppendNewSockFdIntoEpfd(int sockfd,
                                            SocketInfo::ESockType socktype,
                                            uint32_t epevents) {
  struct epoll_event *epev = new struct epoll_event;
  epev->data.ptr = new SocketInfo();
  static_cast<SocketInfo *>(epev->data.ptr)->fd = sockfd;
  static_cast<SocketInfo *>(epev->data.ptr)->socktype = socktype;
  static_cast<SocketInfo *>(epev->data.ptr)->phase =
      SocketInfo::Request;  // 新規接続は皆Requestから始まる
  epev->events = epevents;
  return epoll_ctl(epfd_, EPOLL_CTL_ADD, sockfd, epev) == 0;
}

}  // namespace server
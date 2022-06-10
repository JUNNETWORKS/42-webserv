#include "socket_manager.hpp"

#include <sys/epoll.h>
#include <unistd.h>

#include "server/event_loop.hpp"
#include "server/types.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {

SocketManager::SocketManager(Epoll &epoll) : epoll_(epoll) {}

SocketManager::~SocketManager() {}

bool SocketManager::AppendListenFd(int fd, const std::string &port) {
  if (socket_fd_map_.find(fd) != socket_fd_map_.end()) {
    return false;
  }
  return AppendSocketIntoEpoll(fd, port, SocketInfo::ListenSock, EPOLLIN);
}

bool SocketManager::AppendListenFd(const ListenFdPortMap &listen_fd_port_map) {
  for (ListenFdPortMap::const_iterator it = listen_fd_port_map.begin();
       it != listen_fd_port_map.end(); ++it) {
    if (!AppendListenFd(it->first, it->second)) {
      return false;
    }
  }
  return true;
}

bool SocketManager::AcceptNewConnection(int listen_fd) {
  if (socket_fd_map_.find(listen_fd) == socket_fd_map_.end() ||
      socket_fd_map_.find(listen_fd)->second.socktype !=
          SocketInfo::ListenSock) {
    return false;
  }
  SocketInfo &listen_socket_info = socket_fd_map_.find(listen_fd)->second;

  struct sockaddr_storage client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
  fcntl(conn_fd, F_SETFD, O_NONBLOCK);

  std::string listen_fd_port = socket_fd_map_[listen_fd].port;

  if (!AppendSocketIntoEpoll(conn_fd, listen_socket_info.port,
                             SocketInfo::ConnSock, EPOLLIN | EPOLLOUT)) {
    return false;
  }
  utils::LogConnectionInfoToStdout(client_addr);
  return true;
}

void SocketManager::CloseConnFd(int fd) {
  close(fd);
  epoll_.RemoveFd(fd);
  socket_fd_map_.erase(fd);
}

SocketInfo *SocketManager::GetSocketInfo(int fd) {
  return &socket_fd_map_[fd];
}

bool SocketManager::IsListenFd(int fd) {
  std::map<int, SocketInfo>::const_iterator socket_fd_it =
      socket_fd_map_.find(fd);
  return socket_fd_it != socket_fd_map_.end() &&
         socket_fd_it->second.socktype == SocketInfo::ListenSock;
}

bool SocketManager::IsConnFd(int fd) {
  std::map<int, SocketInfo>::const_iterator socket_fd_it =
      socket_fd_map_.find(fd);
  return socket_fd_it != socket_fd_map_.end() &&
         socket_fd_it->second.socktype == SocketInfo::ConnSock;
}

Epoll &SocketManager::GetEpoll() {
  return epoll_;
}

bool SocketManager::AppendSocketIntoEpoll(int sockfd, const std::string &port,
                                          SocketInfo::ESockType socktype,
                                          unsigned int epevents) {
  socket_fd_map_[sockfd].fd = sockfd;
  socket_fd_map_[sockfd].port = port;
  socket_fd_map_[sockfd].socktype = socktype;

  FdEvent file_descriptor(sockfd, FdEvent::SocketFd, NULL);

  return epoll_.AddFd(file_descriptor, epevents);
}

}  // namespace server

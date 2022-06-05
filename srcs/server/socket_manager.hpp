#ifndef SERVER_SOCKET_MANAGER_HPP
#define SERVER_SOCKET_MANAGER_HPP

#include <sys/epoll.h>

#include <ctime>
#include <vector>

#include "server/socket_info.hpp"
#include "server/types.hpp"

namespace server {

class SocketManager {
 private:
  const int epfd_;

  // map[<listen_fd>] = <port_number>
  ListenFdPortMap listen_fd_port_map_;

 public:
  SocketManager();

  ~SocketManager();

  bool AppendListenFd(int fd, const std::string &port);

  bool AppendListenFd(const ListenFdPortMap &listen_fd_port_map);

  // listen_fd に来た新しい接続要求を受理し､epollに追加する｡
  bool AcceptNewConnection(int listen_fd);

  // close(fd) を行い､ epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) を行う｡
  bool CloseConnFd(int fd);

  // epfd で利用可能なイベントを1つ取得する.
  struct epoll_event WaitEvent();

  // ========================================================================
  // Getter and Setter

  int GetEpollFd() const;

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  SocketManager(const SocketManager &rhs);
  SocketManager &operator=(const SocketManager &rhs);

  bool AppendNewSockFdIntoEpfd(int sockfd, SocketInfo::ESockType socktype,
                               unsigned int epevents);
};

}  // namespace server

#endif

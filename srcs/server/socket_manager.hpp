#ifndef SERVER_SOCKET_MANAGER_HPP
#define SERVER_SOCKET_MANAGER_HPP
#include <ctime>
#include <vector>

#include "server/event_loop.hpp"

namespace server {

class SocketManager {
 private:
  const int epfd_;

 public:
  SocketManager();

  ~SocketManager();

  bool AppendListenFd(int fd);

  bool AppendListenFd(const std::vector<int> &fd_vec);

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
  // epoll instance がcloseされるのを防ぐためコピー操作は禁止
  SocketManager(const SocketManager &rhs);
  SocketManager &operator=(const SocketManager &rhs);

  bool AppendNewSockFdIntoEpfd(int sockfd, SocketInfo::ESockType socktype,
                               uint32_t epevents);
};

}  // namespace server

#endif

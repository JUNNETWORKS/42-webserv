#ifndef SERVER_SOCKET_MANAGER_HPP
#define SERVER_SOCKET_MANAGER_HPP
#include <ctime>
#include <vector>

#include "server/event_loop.hpp"

namespace server {

class SocketManager {
 private:
  int epfd_;

 public:
  SocketManager();

  SocketManager(const SocketManager &rhs);

  SocketManager &operator=(const SocketManager &rhs);

  ~SocketManager();

  bool AppendListenFd(int fd);

  bool AppendListenFd(const std::vector<int> &fd_vec);

  // listen_fd に来た新しい接続要求を受理し､epollに追加する｡
  bool AcceptNewConnection(int listen_fd);

  // close(fd) を行い､ epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) を行う｡
  bool CloseConnFd(int fd);

  // epfd で利用可能なイベントを1つ取得する.
  struct epoll_event WaitEvent();

  // タイムアウトしたソケットを削除する

  // ========================================================================
  // Getter and Setter

  int GetEpollFd() const;

 private:
  bool AppendNewSockFdIntoEpfd(int sockfd, SocketInfo::ESockType socktype,
                               uint32_t epevents);
};

}  // namespace server

#endif

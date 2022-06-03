#ifndef SERVER_EVENT_MANAGER_HPP
#define SERVER_EVENT_MANAGER_HPP
#include <vector>

#include "server/event_loop.hpp"

namespace server {

class EventManager {
 private:
  int epfd_;

 public:
  EventManager();

  EventManager(const EventManager &rhs);

  EventManager &operator=(const EventManager &rhs);

  ~EventManager();

  bool AppendListenFd(int fd);

  bool AppendListenFd(const std::vector<int> &fd_vec);

  // listen_fd に来た新しい接続要求を受理し､epollに追加する｡
  bool AcceptNewConnection(int listen_fd);

  // epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) を行う｡
  // close(fd) を行わないことに注意｡
  bool RemoveFd(int fd);

  // epfd で利用可能なイベントを1つ取得する.
  struct epoll_event WaitEvent();

  // ========================================================================
  // Getter and Setter

  int GetEpollFd() const;

 private:
  bool AppendNewSockFdIntoEpfd(int sockfd, SocketInfo::ESockType socktype,
                               uint32_t epevents);
};

}  // namespace server

#endif

#ifndef SERVER_SOCKET_MANAGER_HPP
#define SERVER_SOCKET_MANAGER_HPP

#include <sys/epoll.h>

#include <ctime>
#include <map>
#include <vector>

#include "server/epoll.hpp"
#include "server/socket_info.hpp"
#include "server/types.hpp"

namespace server {

class SocketManager {
 private:
  Epoll epoll_;

  // map[<sock_fd>] = <socket_info>
  std::map<int, SocketInfo> socket_fd_map_;

 public:
  SocketManager();

  ~SocketManager();

  bool AppendListenFd(int fd, const std::string &port);

  bool AppendListenFd(const ListenFdPortMap &listen_fd_port_map);

  // listen_fd に来た新しい接続要求を受理し､epollに追加する｡
  bool AcceptNewConnection(int listen_fd);

  void CloseConnFd(int fd);

  SocketInfo *GetSocketInfo(int fd);

  bool IsListenFd(int fd);

  bool IsConnFd(int fd);

  Epoll &GetEpoll();

  bool WaitEvents(std::vector<struct epoll_event> &events);

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  SocketManager(const SocketManager &rhs);
  SocketManager &operator=(const SocketManager &rhs);

  bool AppendSocketIntoEpoll(int sockfd, const std::string &port,
                             SocketInfo::ESockType socktype,
                             unsigned int epevents);
};

}  // namespace server

#endif

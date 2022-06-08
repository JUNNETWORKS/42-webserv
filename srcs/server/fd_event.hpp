#ifndef SERVER_FILEDESCRIPTOR_HPP_
#define SERVER_FILEDESCRIPTOR_HPP_

// Epoll クラスで利用する｡
// fd の種類と発生したイベントの種類を保持する構造体
struct FdEvent {
  enum EFdType { SocketFd };
  int fd;
  EFdType fd_type;
  void *ptr;

  // epoll events
  unsigned int events;

  FdEvent(int fd, EFdType fd_type, void *ptr);
};

#endif

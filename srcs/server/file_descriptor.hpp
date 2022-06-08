#ifndef SERVER_FILEDESCRIPTOR_HPP_
#define SERVER_FILEDESCRIPTOR_HPP_

struct FileDescriptor {
  enum EFdType { SocketFd };
  int fd;
  EFdType fd_type;
  void *ptr;

  FileDescriptor(int fd, EFdType fd_type, void *ptr);
};

#endif

#include "server/event_loop.hpp"

#include "config/config.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {
namespace {

const int BUF_SIZE = 1024;

int AppendNewSockFdIntoEpfd(int epfd, int sockfd,
                            SocketInfo::ESockType socktype, uint32_t epevents) {
  struct epoll_event *epev = new struct epoll_event;
  epev->data.ptr = new SocketInfo();
  static_cast<SocketInfo *>(epev->data.ptr)->fd = sockfd;
  static_cast<SocketInfo *>(epev->data.ptr)->socktype = socktype;
  static_cast<SocketInfo *>(epev->data.ptr)->phase =
      SocketInfo::Request;  // 新規接続は皆Requestから始まる
  epev->events = epevents;
  return epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, epev);
}

void LogConnectionInfoToStdout(struct sockaddr_storage &client_addr) {
  socklen_t len = sizeof(struct sockaddr_storage);
  char addrStr[utils::IS_ADDR_STR_LEN];
  utils::inetAddressStr((struct sockaddr *)&client_addr, len, addrStr,
                        utils::IS_ADDR_STR_LEN);
  printf("Connection from %s\n", addrStr);
}

void AcceptNewConnection(int epfd, int listen_fd) {
  struct sockaddr_storage client_addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
  fcntl(conn_fd, F_SETFD, O_NONBLOCK);
  if (AppendNewSockFdIntoEpfd(epfd, conn_fd, SocketInfo::ConnSock, EPOLLIN) ==
      -1) {
    utils::ErrExit("AddSocketFdIntoEpFd");
  }
  LogConnectionInfoToStdout(client_addr);
}

}  // namespace

int StartEventLoop(const std::vector<int> &listen_fds,
                   const config::Config &config) {
  // epoll インスタンス作成
  int epfd = epoll_create1(EPOLL_CLOEXEC);

  // epfd に listen_fd を追加
  for (std::vector<int>::const_iterator it = listen_fds.begin();
       it != listen_fds.end(); ++it) {
    AppendNewSockFdIntoEpfd(epfd, *it, SocketInfo::ListenSock, EPOLLIN);
  }

  // イベントループ
  struct epoll_event epevarr[1];
  std::cout << "start worker" << std::endl;
  while (1) {
    // epfd で利用可能なイベントを1つ取得する
    if (epoll_wait(epfd, epevarr, 1, -1) == -1) {
      if (errno == EINTR)
        continue;  // wait epoll again if interrupted by signal
      else
        exit(EXIT_FAILURE);
    }

    SocketInfo *socket_info = static_cast<SocketInfo *>(epevarr[0].data.ptr);
    if (socket_info->socktype == SocketInfo::ListenSock) {
      int listen_fd = socket_info->fd;
      AcceptNewConnection(epfd, listen_fd);
    } else {
      int conn_fd = socket_info->fd;

      // TODO: Phase によって処理を切り替える
      // TODO: 文字ごと､または行ごとに

      // if data in read buffer, read
      if (epevarr[0].events & EPOLLIN) {
        unsigned char buf[BUF_SIZE];
        int n = read(conn_fd, buf, sizeof(buf) - 1);
        if (n <= 0) {  // EOF(Connection end) or Error
          printf("Connection end\n");
          close(conn_fd);
          epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, NULL);  // 明示的に消してる
        } else {
          socket_info->request.AppendDataToBuffer(buf, n);
          printf("----- Received data -----\n%s", buf);
        }
      }

      // if space in write buffer, read
      if (epevarr[0].events & EPOLLOUT) {
        // TODO: Send HTTP Response to the client
      }

      // error or timeout? close conn_fd and remove from epfd
      if (epevarr[0].events & (EPOLLERR | EPOLLHUP)) {
        printf("Connection error\n");
        close(conn_fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, NULL);  // 明示的に消してる
      }
    }
  }
}

};  // namespace server

#include "server/event_loop.hpp"

#include "config/config.hpp"
#include "server/socket_manager.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {
namespace {

const int BUF_SIZE = 1024;

void ProcessRequest(SocketManager &socket_manager, SocketInfo *info) {
  unsigned char buf[BUF_SIZE];
  int conn_fd = info->fd;
  int n = read(conn_fd, buf, sizeof(buf) - 1);
  if (n <= 0) {  // EOF(Connection end) or Error
                 // TODO ここいらないかも？
    printf("Connection end\n");
    socket_manager.CloseConnFd(conn_fd);
  } else {
    info->buffer_.AppendDataToBuffer(buf, n);

    while (1) {
      if (info->requests.empty() || info->requests.back().IsParsed()) {
        info->requests.push_back(http::HttpRequest());
      }
      info->requests.back().ParseRequest(info->buffer_);
      if (info->requests.back().IsCorrectStatus() == false) {
        info->buffer_.clear();
      }
      if (info->buffer_.empty() || info->requests.back().IsParsed() == false) {
        break;
      }
    }
  }
}

void ProcessResponse(SocketManager &socket_manager, SocketInfo *info) {
  // TODO: Send HTTP Response to the client
  int conn_fd = info->fd;
  if (info->requests.front().IsCorrectRequest()) {
    info->response.SetStatusLine("HTTP/1.1 200 OK");
    info->response.LoadFile(info->requests.front().GetPath());
    info->response.Write(conn_fd);
    socket_manager.CloseConnFd(conn_fd);
  } else {
    socket_manager.CloseConnFd(conn_fd);
  }
}

}  // namespace

int StartEventLoop(const std::vector<int> &listen_fds,
                   const config::Config &config) {
  // TODO: configを利用するようにする
  (void)config;

  // epoll インスタンス作成
  SocketManager socket_manager;

  socket_manager.AppendListenFd(listen_fds);

  // イベントループ
  while (1) {
    struct epoll_event epev = socket_manager.WaitEvent();
    SocketInfo *socket_info = static_cast<SocketInfo *>(epev.data.ptr);

    if (socket_info->socktype == SocketInfo::ListenSock) {
      if (!socket_manager.AcceptNewConnection(socket_info->fd)) {
        utils::ErrExit("AddSocketFdIntoEpFd");
      }
    } else {
      int conn_fd = socket_info->fd;
      // if data in read buffer, read
      if (epev.events & EPOLLIN) {
        ProcessRequest(socket_manager, socket_info);
      }
      // if space in write buffer, read
      if (epev.events & EPOLLOUT) {
        ProcessResponse(socket_manager, socket_info);
      }

      // error or timeout? close conn_fd and remove from epfd
      if (epev.events & (EPOLLERR | EPOLLHUP)) {
        printf("Connection error\n");
        socket_manager.CloseConnFd(conn_fd);
      }
    }
  }
}

}  // namespace server

#include "server/event_loop.hpp"

#include "config/config.hpp"
#include "server/event_manager.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {
namespace {

const int BUF_SIZE = 1024;

void ProcessRequest(int conn_fd, int epfd, SocketInfo *info) {
  unsigned char buf[BUF_SIZE];
  int n = read(conn_fd, buf, sizeof(buf) - 1);
  if (n <= 0) {  // EOF(Connection end) or Error
                 // TODO ここいらないかも？
    printf("Connection end\n");
    close(conn_fd);
    epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, NULL);  // 明示的に消してる
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

}  // namespace

int StartEventLoop(const std::vector<int> &listen_fds,
                   const config::Config &config) {
  // TODO: configを利用するようにする
  (void)config;

  // epoll インスタンス作成
  EventManager event_manager;

  event_manager.AppendListenFd(listen_fds);

  // イベントループ
  while (1) {
    struct epoll_event epev = event_manager.WaitEvent();
    SocketInfo *socket_info = static_cast<SocketInfo *>(epev.data.ptr);

    if (socket_info->socktype == SocketInfo::ListenSock) {
      if (!event_manager.AcceptNewConnection(socket_info->fd)) {
        utils::ErrExit("AddSocketFdIntoEpFd");
      }
    } else {
      int conn_fd = socket_info->fd;
      // if data in read buffer, read
      if (epev.events & EPOLLIN) {
        ProcessRequest(conn_fd, event_manager.GetEpollFd(), socket_info);
      }
      // if space in write buffer, read
      if (epev.events & EPOLLOUT) {
        // TODO: Send HTTP Response to the client
        if (socket_info->requests.front().IsCorrectRequest()) {
          socket_info->response.SetStatusLine("HTTP/1.1 200 OK");
          // socket_info->response.setHeader("Content-Type: text/plain");
          socket_info->response.LoadFile(
              socket_info->requests.front().GetPath());
          socket_info->response.Write(conn_fd);
          close(conn_fd);
        } else {
          close(conn_fd);
        }
      }

      // error or timeout? close conn_fd and remove from epfd
      if (epev.events & (EPOLLERR | EPOLLHUP)) {
        printf("Connection error\n");
        close(conn_fd);
        event_manager.RemoveFd(conn_fd);  // 明示的に消してる
      }
    }
  }
}

}  // namespace server

#include "server/event_loop.hpp"

#include "config/config.hpp"
#include "server/socket_info.hpp"
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

void ProcessResponse(SocketManager &socket_manager, SocketInfo *info,
                     const config::Config &config) {
  int conn_fd = info->fd;
  http::HttpRequest &request = info->requests.front();
  if (!info->requests.empty() && request.IsCorrectRequest()) {
    const std::string &host =
        request.GetHeader("Host").empty() ? "" : request.GetHeader("Host")[0];
    const std::string &port = info->port;

    // ポートとHostヘッダーから VirtualServerConf を取得
    const config::VirtualServerConf *vserver =
        config.GetVirtualServerConf(port, host);
    if (!vserver) {
      // 404 Not Found を返す
      info->response.MakeErrorResponse(NULL, request, http::NOT_FOUND);
      info->response.Write(conn_fd);
      socket_manager.CloseConnFd(conn_fd);
      return;
    }
    printf("===== Virtual Server =====\n");
    vserver->Print();

    info->response.MakeResponse(*vserver, request);
    info->response.Write(conn_fd);
    socket_manager.CloseConnFd(conn_fd);
  }
}

}  // namespace

int StartEventLoop(SocketManager &socket_manager,
                   const config::Config &config) {
  std::vector<struct epoll_event> epoll_events;
  // イベントループ
  while (1) {
    epoll_events.clear();
    if (!socket_manager.WaitEvents(epoll_events)) {
      utils::ErrExit("WaitEvents");
    }

    for (std::vector<struct epoll_event>::const_iterator it =
             epoll_events.begin();
         it != epoll_events.end(); ++it) {
      int fd = it->data.fd;
      unsigned int event_type = it->events;
      SocketInfo *socket_info = socket_manager.GetSocketInfo(fd);

      if (socket_info->socktype == SocketInfo::ListenSock) {
        if (!socket_manager.AcceptNewConnection(socket_info->fd)) {
          utils::ErrExit("AddSocketFdIntoEpFd");
        }
      } else {
        int conn_fd = socket_info->fd;
        // if data in read buffer, read
        if (event_type & EPOLLIN) {
          ProcessRequest(socket_manager, socket_info);
        }
        // if space in write buffer, read
        if (event_type & EPOLLOUT) {
          ProcessResponse(socket_manager, socket_info, config);
        }

        // error or timeout? close conn_fd and remove from epfd
        if (event_type & (EPOLLERR | EPOLLHUP)) {
          printf("Connection error\n");
          socket_manager.CloseConnFd(conn_fd);
        }
      }
    }
  }
}

}  // namespace server

#include "server/socket_event_handler.hpp"

#include <unistd.h>

#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"

namespace server {

namespace {

bool ProcessRequest(Socket *socket);
bool ProcessResponse(Socket *socket);

}  // namespace

void HandleConnSocketEvent(int fd, unsigned int events, void *data,
                           Epoll *epoll) {
  Socket *socket = reinterpret_cast<Socket *>(data);
  bool should_close_conn = false;

  // if data in read buffer, read
  if (events & EPOLLIN) {
    should_close_conn = ProcessRequest(socket);
  }
  // if space in write buffer, read
  if (events & EPOLLOUT) {
    should_close_conn = ProcessResponse(socket);
  }

  // error or timeout? close conn_fd and remove from epfd
  if (events & (EPOLLERR | EPOLLHUP) || should_close_conn) {
    printf("Connection close\n");
    epoll->RemoveFd(fd);
  }
}

void HandleListenSocketEvent(int fd, unsigned int events, void *data,
                             Epoll *epoll) {
  Socket *socket = reinterpret_cast<Socket *>(data);

  if (events | EPOLLIN) {
    Result<Socket *> result = socket->AcceptNewConnection();
    if (result.IsErr()) {
      // 本当はログ出力とかがあると良い｡
      return;
    }
    Socket *conn_sock = result.Ok();
    FdEvent *fdevent =
        CreateFdEvent(conn_sock->GetFd(), HandleConnSocketEvent, conn_sock);
    epoll->AddFd(fdevent, EPOLLIN | EPOLLOUT);
  }

  // error or timeout? close conn_fd and remove from epfd
  if (events & (EPOLLERR | EPOLLHUP)) {
    // TODO:
    epoll->RemoveFd(fd);
  }
}

namespace {
const int BUF_SIZE = 1024;

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessRequest(Socket *socket) {
  unsigned char buf[BUF_SIZE];
  int conn_fd = socket->GetFd();
  int n = read(conn_fd, buf, sizeof(buf) - 1);
  if (n <= 0) {  // EOF(Connection end) or Error
                 // TODO ここいらないかも？
    printf("Connection end\n");
    // 呼び出し元でepollから対象conn_fdを削除する｡
    return true;
  } else {
    utils::ByteVector &buffer = socket->GetBuffer();
    buffer.AppendDataToBuffer(buf, n);

    while (1) {
      std::vector<http::HttpRequest> &requests = socket->GetRequests();
      if (requests.empty() || requests.back().IsParsed()) {
        requests.push_back(http::HttpRequest());
      }
      requests.back().ParseRequest(buffer);
      if (requests.back().IsCorrectStatus() == false) {
        buffer.clear();
      }
      if (buffer.empty() || requests.back().IsParsed() == false) {
        break;
      }
    }
  }
  return false;
}

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessResponse(Socket *socket) {
  int conn_fd = socket->GetFd();
  const config::Config &config = socket->GetConfig();

  if (!socket->GetRequests().empty() &&
      socket->GetRequests().front().IsCorrectRequest()) {
    http::HttpRequest &request = socket->GetRequests().front();
    http::HttpResponse &response = socket->GetResponse();
    const std::string &host =
        request.GetHeader("Host").empty() ? "" : request.GetHeader("Host")[0];
    const std::string &port = socket->GetPort();

    // ポートとHostヘッダーから VirtualServerConf を取得
    const config::VirtualServerConf *vserver =
        config.GetVirtualServerConf(port, host);
    if (!vserver) {
      // 404 Not Found を返す
      response.MakeErrorResponse(NULL, request, http::NOT_FOUND);
      response.Write(conn_fd);
      return true;
    }
    printf("===== Virtual Server =====\n");
    vserver->Print();

    response.MakeResponse(*vserver, request);
    response.Write(conn_fd);
  }
  // TODO: 複数リクエストに対応する際には､
  //       リクエストの状態によってcloseするかどうか判断して返す｡
  // 現在はレスポンスを write したら常にソケットをcloseするようにしている｡
  return true;
}

}  // namespace

}  // namespace server

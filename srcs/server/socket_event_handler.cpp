#include "server/socket_event_handler.hpp"

#include <unistd.h>

#include <deque>

#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"
#include "utils/error.hpp"

namespace server {

namespace {

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessRequest(ConnSocket *socket);

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessResponse(ConnSocket *socket);

// HTTPリクエストのヘッダーに "Connection: close" が含まれているか
//
// request を const_reference で受け取っていないのは
// HttpRequest.GetHeader() が const メソッドじゃないから
bool RequestHeaderHasConnectionClose(http::HttpRequest &request);

}  // namespace

void HandleConnSocketEvent(FdEvent *fde, unsigned int events, void *data,
                           Epoll *epoll) {
  ConnSocket *conn_sock = reinterpret_cast<ConnSocket *>(data);
  bool should_close_conn = false;

  if (events & kFdeRead) {
    should_close_conn |= ProcessRequest(conn_sock);
  }
  if (events & kFdeWrite) {
    should_close_conn |= ProcessResponse(conn_sock);
  }

  if (conn_sock->HasParsedRequest()) {
    epoll->Add(fde, kFdeWrite);
  } else {
    epoll->Del(fde, kFdeWrite);
  }

  if (should_close_conn || (events & kFdeTimeout)) {
    printf("Connection close\n");
    epoll->Unregister(fde);
    // conn_sock->fd の close は Socket のデストラクタで行うので不要｡
    delete conn_sock;
    delete fde;
  }
}

void HandleListenSocketEvent(FdEvent *fde, unsigned int events, void *data,
                             Epoll *epoll) {
  (void)fde;
  ListenSocket *listen_sock = reinterpret_cast<ListenSocket *>(data);

  if (events & kFdeRead) {
    Result<ConnSocket *> result = listen_sock->AcceptNewConnection();
    if (result.IsErr()) {
      // 本当はログ出力とかがあると良い｡
      return;
    }
    ConnSocket *conn_sock = result.Ok();
    FdEvent *conn_fde =
        CreateFdEvent(conn_sock->GetFd(), HandleConnSocketEvent, conn_sock);
    epoll->Register(conn_fde);
    epoll->Add(conn_fde, kFdeRead);
    epoll->SetTimeout(conn_fde, ConnSocket::kDefaultTimeoutMs);
  }

  if (events & kFdeError) {
    // Listenしているソケットが正常に動いていない場合はプログラムを終了させる
    utils::ErrExit("ListenSocket(port %s) occur error\n",
                   listen_sock->GetPort().c_str());
  }
}

namespace {
const int BUF_SIZE = 1024;

bool ProcessRequest(ConnSocket *socket) {
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
      std::deque<http::HttpRequest> &requests = socket->GetRequests();
      if (requests.empty() || requests.back().IsParsed()) {
        requests.push_back(http::HttpRequest(socket->GetConfig()));
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

bool RequestHeaderHasConnectionClose(http::HttpRequest &request) {
  const std::vector<std::string> &connection_header =
      request.GetHeader("Connection");
  for (std::vector<std::string>::const_iterator it = connection_header.begin();
       it != connection_header.end(); ++it) {
    if (*it == "close") {
      return true;
    }
  }
  return false;
}

bool ProcessResponse(ConnSocket *socket) {
  int conn_fd = socket->GetFd();
  const config::Config &config = socket->GetConfig();
  std::deque<http::HttpRequest> &requests = socket->GetRequests();
  bool should_close_conn = false;

  if (socket->HasParsedRequest()) {
    http::HttpRequest &request = requests.front();
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
      response.Clear();
      return should_close_conn;
    }

    response.MakeResponse(*vserver, request);
    response.Write(conn_fd);
    response.Clear();

    // "Connection: close" がリクエストで指定されていた場合はソケット接続を切断
    should_close_conn = RequestHeaderHasConnectionClose(request);

    requests.pop_front();
  }

  return should_close_conn;
}

}  // namespace

}  // namespace server

#include "server/socket_event_handler.hpp"

#include <unistd.h>

#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"
#include "utils/error.hpp"

namespace server {

namespace {

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessRequest(Socket *socket);

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessResponse(Socket *socket);

}  // namespace

void HandleConnSocketEvent(FdEvent *fde, unsigned int events, void *data,
                           Epoll *epoll) {
  Socket *conn_sock = reinterpret_cast<Socket *>(data);
  bool should_close_conn = false;

  if (events & kFdeRead) {
    should_close_conn |= ProcessRequest(conn_sock);
  }
  if (events & kFdeWrite) {
    should_close_conn |= ProcessResponse(conn_sock);
  }
  if (events & kFdeTimeout) {
    // TODO: タイムアウト処理
  }

  // TCP FIN が送信したデータより早く来る場合があり､
  // その対策として kFdeError で接続切断をするのではなく､
  // read(conn_fd) の返り値が0(EOF)または-1(Error)だったら切断する｡
  if (should_close_conn) {
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
  Socket *listen_sock = reinterpret_cast<Socket *>(data);

  if (events & kFdeRead) {
    Result<Socket *> result = listen_sock->AcceptNewConnection();
    if (result.IsErr()) {
      // 本当はログ出力とかがあると良い｡
      return;
    }
    Socket *conn_sock = result.Ok();
    FdEvent *fdevent =
        CreateFdEvent(conn_sock->GetFd(), HandleConnSocketEvent, conn_sock);
    epoll->Register(fdevent);
    epoll->Add(fdevent, kFdeRead);
    epoll->Add(fdevent, kFdeWrite);
  }

  if (events & kFdeError) {
    // Listenしているソケットが正常に動いていない場合はプログラムを終了させる
    utils::ErrExit("ListenSocket(port %s) occur error\n",
                   listen_sock->GetPort().c_str());
  }
}

namespace {
const int BUF_SIZE = 1024;

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

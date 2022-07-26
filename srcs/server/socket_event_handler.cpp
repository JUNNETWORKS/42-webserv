#include "server/socket_event_handler.hpp"

#include <unistd.h>

#include <deque>

#include "http/http_cgi_response.hpp"
#include "http/http_response.hpp"
#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"
#include "utils/error.hpp"

namespace server {

namespace {

// リクエストに応じたレスポンスを返す
http::HttpResponse *AllocateResponseObj(const http::HttpRequest &request,
                                        Epoll *epoll);

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessRequest(ConnSocket *socket);

// 呼び出し元でソケットを閉じる必要がある場合は true を返す
bool ProcessResponse(ConnSocket *socket, Epoll *epoll);

// HTTPレスポンスのヘッダーに "Connection: close" が含まれているか
//
// request を const_reference で受け取っていないのは
// HttpResponse.GetHeader() が const メソッドじゃないから
bool ResponseHeaderHasConnectionClose(http::HttpResponse &response);

}  // namespace

void HandleConnSocketEvent(FdEvent *fde, unsigned int events, void *data,
                           Epoll *epoll) {
  ConnSocket *conn_sock = reinterpret_cast<ConnSocket *>(data);
  bool should_close_conn = false;

  if (events & kFdeRead) {
    should_close_conn |= ProcessRequest(conn_sock);
  }
  if (events & kFdeWrite) {
    should_close_conn |= ProcessResponse(conn_sock, epoll);
  }

  if (conn_sock->HasParsedRequest()) {
    epoll->Add(fde, kFdeWrite);
  } else {
    epoll->Del(fde, kFdeWrite);
  }

  if ((should_close_conn && conn_sock->IsShutdown()) ||
      (events & kFdeTimeout)) {
    printf("Connection close\n");
    epoll->Unregister(fde);
    // conn_sock->fd の close は Socket のデストラクタで行うので不要｡
    delete conn_sock;
    delete fde;
  } else if (should_close_conn && !conn_sock->IsShutdown()) {
    conn_sock->ShutDown();
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
                   listen_sock->GetServerPort().c_str());
  }
}

namespace {
const int BUF_SIZE = 1024;

bool ProcessRequest(ConnSocket *socket) {
  unsigned char buf[BUF_SIZE];
  int conn_fd = socket->GetFd();
  int n = read(conn_fd, buf, sizeof(buf) - 1);
  if (n <= 0) {  // EOF(TCP flag FIN) or Error
    printf("Connection end\n");
    socket->SetIsShutdown(true);
    return true;
  } else {
    utils::ByteVector &buffer = socket->GetBuffer();
    buffer.AppendDataToBuffer(buf, n);

    while (1) {
      std::deque<http::HttpRequest> &requests = socket->GetRequests();
      if (requests.empty() || requests.back().IsResponsible()) {
        requests.push_back(http::HttpRequest());
      }
      requests.back().ParseRequest(buffer, socket->GetConfig(),
                                   socket->GetServerPort());
      if (requests.back().IsErrorRequest()) {
        buffer.clear();
        break;
      }
      if (buffer.empty() || requests.back().IsResponsible() == false) {
        break;
      }
    }
  }
  return false;
}

bool ResponseHeaderHasConnectionClose(http::HttpResponse &response) {
  const std::vector<std::string> &connection_header =
      response.GetHeader("Connection");
  for (std::vector<std::string>::const_iterator it = connection_header.begin();
       it != connection_header.end(); ++it) {
    if (*it == "close") {
      return true;
    }
  }
  return false;
}

bool ProcessResponse(ConnSocket *socket, Epoll *epoll) {
  int conn_fd = socket->GetFd();
  std::deque<http::HttpRequest> &requests = socket->GetRequests();
  bool should_close_conn = false;

  if (socket->HasParsedRequest()) {
    http::HttpRequest &request = requests.front();

    if (socket->GetResponse() == NULL) {
      // レスポンスオブジェクトがまだない
      http::HttpResponse *response = AllocateResponseObj(request, epoll);
      socket->SetResponse(response);
    }

    http::HttpResponse *response = socket->GetResponse();
    should_close_conn |= response->PrepareToWrite(socket).IsErr();
    if (!should_close_conn && response->IsAllDataWritingCompleted() == false) {
      // 書き込むデータが存在する
      should_close_conn |= response->WriteToSocket(conn_fd).IsErr();
    }
    if (!should_close_conn && response->IsAllDataWritingCompleted()) {
      // "Connection: close"
      // がレスポンスヘッダーに存在していればソケット接続を切断
      should_close_conn |= ResponseHeaderHasConnectionClose(*response);
      delete response;
      socket->SetResponse(NULL);
      requests.pop_front();
    }
  }

  return should_close_conn;
}

http::HttpResponse *AllocateResponseObj(const http::HttpRequest &request,
                                        Epoll *epoll) {
  const config::LocationConf *location = request.GetLocation();
  if (request.IsErrorRequest())
    return new http::HttpResponse(location, epoll, request.GetParseStatus());
  if (location->GetIsCgi()) {
    return new http::HttpCgiResponse(location, epoll);
  } else {
    return new http::HttpResponse(location, epoll);
  }
}

}  // namespace

}  // namespace server

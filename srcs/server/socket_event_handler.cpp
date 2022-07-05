#include "server/socket_event_handler.hpp"

#include <unistd.h>

#include <deque>

#include "http/http_response.hpp"
#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"
#include "utils/error.hpp"

namespace server {

namespace {

// リクエストに応じたレスポンスを返す
http::HttpResponse *AllocateResponseObj(
    const config::VirtualServerConf *vserver, const http::HttpRequest &request,
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

bool RequestHeaderHasConnectionClose(http::HttpRequest &request) {
  Result<const std::vector<std::string> &> connection_header_res =
      request.GetHeader("Connection");
  if (connection_header_res.IsErr())
    return false;
  const std::vector<std::string> connection_header = connection_header_res.Ok();

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
  const config::Config &config = socket->GetConfig();
  std::deque<http::HttpRequest> &requests = socket->GetRequests();
  bool should_close_conn = false;

  if (socket->HasParsedRequest()) {
    http::HttpRequest &request = requests.front();
    http::HttpResponse &response = socket->GetResponse();
    const std::string &host = request.GetHeader("Host").Ok()[0];
    // TODO vserverをrequestから取得する

    const std::string &port = socket->GetPort();

    // ポートとHostヘッダーから VirtualServerConf を取得
    const config::VirtualServerConf *vserver =
        config.GetVirtualServerConf(port, host);

    if (socket->GetResponse() == NULL) {
      // レスポンスオブジェクトがまだない
      http::HttpResponse *response =
          AllocateResponseObj(vserver, request, epoll);
      socket->SetResponse(response);
      response->MakeResponse(socket);
    }

    http::HttpResponse *response = socket->GetResponse();
    if (response->IsReadyToWrite()) {
      // 書き込むデータが存在する
      should_close_conn |= response->Write(conn_fd).IsErr();
    } else if (response->IsAllDataWritingCompleted()) {
      // "Connection: close"
      // がレスポンスヘッダーに存在していればソケット接続を切断
      should_close_conn |= ResponseHeaderHasConnectionClose(*response);
      // 全て書き込み完了
      delete response;
      socket->SetResponse(NULL);
      requests.pop_front();
    } else {
      // 書き込むデータはないがレスポンスは完成していない
    }
  }

  return should_close_conn;
}

http::HttpResponse *AllocateResponseObj(
    const config::VirtualServerConf *vserver, const http::HttpRequest &request,
    Epoll *epoll) {
  if (!vserver) {
    http::HttpResponse *res = new http::HttpResponse(NULL, epoll);
    res->MakeErrorResponse(request, http::NOT_FOUND);
    return res;
  }
  const config::LocationConf *location =
      vserver->GetLocation(request.GetPath());
  if (!location) {
    http::HttpResponse *res = new http::HttpResponse(NULL, epoll);
    res->MakeErrorResponse(request, http::NOT_FOUND);
    return res;
  }

  return new http::HttpResponse(location, epoll);
  // TODO: 以下のコードが実行できるようにする
  //  if (location->GetIsCgi()) {
  //    return new http::HttpCgiResponse(location, epoll);
  //  } else if (location->GetRedirectUrl()) {
  //    return new http::HttpRedirectResponse(location, epoll);
  //  } else {
  //    return new http::HttpResponse(location, epoll);
  //  }
}

}  // namespace

}  // namespace server

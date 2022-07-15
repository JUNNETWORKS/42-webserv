#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <deque>
#include <string>
#include <vector>

#include "config/config.hpp"
#include "http/http_request.hpp"
#include "http/http_response.hpp"
#include "result/result.hpp"
#include "server/socket_address.hpp"
#include "utils/ByteVector.hpp"

namespace http {
class HttpResponse;
}

namespace server {
using namespace result;

// listen_fd の情報などを持たせたい｡
// CGI でリクエスト元IPなど色々情報が必要になってくるので,
// それらの情報をもたせるようにしたい｡
class Socket {
 protected:
  int fd_;

  // port_ は socktype_ によって意味が変わる｡
  // ListenSock の場合は Listen しているソケット情報
  // ConnSock の場合はAcceptされたListenSockのソケット情報
  const SocketAddress server_addr_;

  const config::Config &config_;

 public:
  Socket(int fd, const SocketAddress &server_addr,
         const config::Config &config);
  Socket(const Socket &rhs);
  // close(fd_) はデストラクタで行われる
  virtual ~Socket() = 0;

  int GetFd() const;

  const std::string &GetServerIp() const;
  const std::string &GetServerPort() const;

  const config::Config &GetConfig() const;

 private:
  Socket();
  Socket &operator=(const Socket &rhs);
};

class ConnSocket : public Socket {
 private:
  const SocketAddress client_addr_;

  std::deque<http::HttpRequest> requests_;
  http::HttpResponse *response_;
  utils::ByteVector buffer_;

 public:
  // タイムアウトのデフォルト時間は5秒
  // HTTP/1.1 では Connection: close が来るまでソケットを接続し続ける｡
  // Nginx などでは5秒間クライアントからデータが来なければ
  //   切断するのでそれに合わせる｡
  static const long kDefaultTimeoutMs = 5 * 1000;

  ConnSocket(int fd, const SocketAddress &server_addr,
             const SocketAddress &client_addr, const config::Config &config);
  virtual ~ConnSocket();

  const std::string &GetRemoteIp() const;
  const std::string &GetRemoteName() const;

  std::deque<http::HttpRequest> &GetRequests();

  bool HasParsedRequest();

  http::HttpResponse *GetResponse();
  void SetResponse(http::HttpResponse *response);

  utils::ByteVector &GetBuffer();

 private:
  ConnSocket();
  ConnSocket &operator=(const ConnSocket &rhs);
};

class ListenSocket : public Socket {
 public:
  ListenSocket(int fd, const SocketAddress &server_addr,
               const config::Config &config);

  // 現在の Socket に来た接続要求を accept する｡
  // 返り値の Socket* はヒープ領域に存在しており､
  // 解放するのは呼び出し側の責任である｡
  Result<ConnSocket *> AcceptNewConnection();

 private:
  ListenSocket();
  ListenSocket &operator=(const ListenSocket &rhs);
};

}  // namespace server

#endif

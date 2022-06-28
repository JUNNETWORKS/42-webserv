#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <deque>
#include <string>
#include <vector>

#include "config/config.hpp"
#include "http/http_request.hpp"
#include "http/http_response.hpp"
#include "result/result.hpp"
#include "utils/ByteVector.hpp"

namespace server {
using namespace result;

// listen_fd の情報などを持たせたい｡
// CGI でリクエスト元IPなど色々情報が必要になってくるので,
// それらの情報をもたせるようにしたい｡
class Socket {
 public:
  enum ESockType { ListenSock, ConnSock };

 protected:
  int fd_;

  ESockType socktype_;

  // port_ は socktype_ によって意味が変わる｡
  // ListenSock の場合は Listen しているポート番号
  // ConnSock の場合はAcceptされたListenSockのポート番号
  const std::string port_;

  const config::Config &config_;

 public:
  Socket(int fd, ESockType socktype, const std::string &port,
         const config::Config &config);
  Socket(const Socket &rhs);
  // close(fd_) はデストラクタで行われる
  virtual ~Socket() = 0;

  int GetFd() const;

  const std::string &GetPort() const;

  const config::Config &GetConfig() const;

  ESockType GetSockType() const;

 private:
  Socket();
  Socket &operator=(const Socket &rhs);
};

class ConnSocket : public Socket {
 private:
  std::deque<http::HttpRequest> requests_;
  http::HttpResponse *response_;
  utils::ByteVector buffer_;

 public:
  // タイムアウトのデフォルト時間は5秒
  // HTTP/1.1 では Connection: close が来るまでソケットを接続し続ける｡
  // Nginx などでは5秒間クライアントからデータが来なければ
  //   切断するのでそれに合わせる｡
  static const long kDefaultTimeoutMs = 5 * 1000;

  ConnSocket(int fd, const std::string &port, const config::Config &config);

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
  ListenSocket(int fd, const std::string &port, const config::Config &config);

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

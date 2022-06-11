#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <string>
#include <vector>

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

 private:
  int fd_;

  ESockType socktype_;

  // port_ は socktype_ によって意味が変わる｡
  // ListenSock の場合は Listen しているポート番号
  // ConnSock の場合はAcceptされたListenSockのポート番号
  std::string port_;

  std::vector<http::HttpRequest> requests_;
  http::HttpResponse response_;
  utils::ByteVector buffer_;

 public:
  Socket(int fd, ESockType socktype, const std::string &port = "");
  Socket(const Socket &rhs);
  Socket &operator=(const Socket &rhs);
  // close(fd_) はデストラクタで行われる
  ~Socket();

  // 現在の Socket に来た接続要求を accept する｡
  // 返り値の Socket* はヒープ領域に存在しており､
  // 解放するのは呼び出し側の責任である｡
  Result<Socket *> AcceptNewConnection();

  // ========================================================================
  // Getter and Setter

  int GetFd() const;

  void SetPort(const std::string &port);
  const std::string &GetPort() const;

  ESockType GetSockType() const;

  utils::ByteVector &GetBuffer();

 private:
  Socket();
};

}  // namespace server

#endif

#ifndef HTTP_HTTP_CGI_RESPONSE_HPP_
#define HTTP_HTTP_CGI_RESPONSE_HPP_

#include "cgi/cgi_process.hpp"
#include "http/http_response.hpp"
#include "http/http_status.hpp"

namespace http {

struct CgiBuffer {
  // 1回のreadで何バイト読み取るか
  static const unsigned long kBytesPerRead = 1024;  // 1KB
  // bufferが保持する最大サイズ｡
  // 1リクエストがメモリを大量に使わないようにするために存在する
  static const unsigned long kMaxBufferSize = 1024 * 50;  // 50KB

  int file_fd;

  // Epoll events
  unsigned int events;
  bool is_eof;

  // このフラグがfalseのときにはdeleteせずにこのフラグをtrueにするだけ｡
  // HttpResponse と EpollEventHandler で double-free しないようにするため
  bool is_unregistered;

  // ファイルから読み取れるときに読み込んでおく
  utils::ByteVector buffer;
};

class HttpCgiResponse : public HttpResponse {
 private:
  cgi::CgiProcess cgi_process_;

 public:
  HttpCgiResponse(const config::LocationConf *location, server::Epoll *epoll);
  virtual ~HttpCgiResponse();

  virtual void MakeResponse(server::ConnSocket *conn_sock);
  virtual Result<void> Write(int fd);

  // データ書き込みが可能か
  virtual bool IsReadyToWrite();

  // すべてのデータの write が完了したか
  virtual bool IsAllDataWritingCompleted();
};
}  // namespace http

#endif

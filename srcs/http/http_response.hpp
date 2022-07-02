#ifndef HTTP_HTTP_RESPONSE_HPP_
#define HTTP_HTTP_RESPONSE_HPP_

#include <map>
#include <string>
#include <vector>

#include "config/virtual_server_conf.hpp"
#include "http/http_request.hpp"
#include "http/http_status.hpp"
#include "http/types.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"
#include "utils/File.hpp"

namespace server {
class ConnSocket;
}

namespace http {

using namespace result;

class HttpResponse {
 protected:
  // 1回のreadで何バイト読み取るか
  static const unsigned long kBytesPerRead = 1024;  // 1KB

  // デフォルトのHTTPバージョン(HTTP/1.1)
  static const std::string kDefaultHttpVersion;

  const config::LocationConf *location_;
  server::Epoll *epoll_;

  // Status Line
  std::string http_version_;
  HttpStatus status_;
  std::string status_message_;

  // Header
  HeaderMap headers_;

  // Status Line と Headers のバイト列と書き込んだバイト数
  utils::ByteVector status_and_headers_bytes_;
  unsigned long writtern_status_headers_count_;

  // Body のバイト列と書き込んだバイト数
  utils::ByteVector body_bytes_;
  unsigned long written_body_count_;

  // File
  // 全てのレスポンスクラスはファイルを返せる必要がある｡
  // なぜならエラー時にファイルを扱う可能性があるからである｡
  int file_fd_;
  bool is_file_eof_;

 public:
  HttpResponse(const config::LocationConf *location, server::Epoll *epoll);
  virtual ~HttpResponse();

  virtual void MakeResponse(server::ConnSocket *conn_sock);

  void MakeErrorResponse(const HttpRequest &request, HttpStatus status);

  virtual Result<void> Write(int fd);

  // データ書き込みが可能か
  virtual bool IsReadyToWrite();

  // すべてのデータの write が完了したか
  virtual bool IsAllDataWritingCompleted();

  const std::vector<std::string> &GetHeader(const std::string &header);

 protected:
  // ファイルをopenし､Epollで監視する
  Result<void> RegisterFile(const std::string &file_path);

  // status-line と header-lines を書き込む｡
  // status_and_headers_bytes_ にデータが無い(初回呼び出し)ときには､
  // データをセットする｡
  //
  // 返り値は今回書き込んだバイト数である｡
  // 0ならば全てのバイト書き込みが完了したことになる｡
  Result<ssize_t> WriteStatusAndHeader(int fd);

  bool IsStatusAndHeadersWritingCompleted();

  // ========================================================================
  // Getter and Setter
  void SetHttpVersion(const std::string &http_version);
  void SetStatus(HttpStatus status);
  void SetStatus(HttpStatus status, const std::string &status_message);
  void SetStatusMessage(const std::string &status_message);
  void SetHeader(const std::string &header, const std::string &value);
  void AppendHeader(const std::string &header, const std::string &value);

 private:
  HttpResponse();
  HttpResponse(const HttpResponse &rhs);
  HttpResponse &operator=(const HttpResponse &rhs);

  // StatusLine と Headers をバイト列にする
  utils::ByteVector SerializeStatusAndHeader() const;
  utils::ByteVector SerializeStatusLine() const;
  utils::ByteVector SerializeHeaders() const;

  std::string MakeErrorResponseBody(HttpStatus status);

  static std::string MakeAutoIndex(const std::string &root_path,
                                   const std::string &relative_path);
};

}  // namespace http

#endif

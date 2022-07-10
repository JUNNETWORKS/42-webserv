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
  // 次何を書き込むか
  enum WritingPhase { kLoadRequest, kStatusAndHeader, kBody, kComplete };

  // 1回のreadで何バイト読み取るか
  static const unsigned long kBytesPerRead = 1024;  // 1KB

  const config::LocationConf *location_;
  server::Epoll *epoll_;

  // 次何を書き込むか
  WritingPhase phase_;

  // デフォルトのHTTPバージョン(HTTP/1.1)
  static const std::string kDefaultHttpVersion;
  static const ssize_t kWriteMaxSize = 1024 * 1024;

  // Status Line
  std::string http_version_;
  HttpStatus status_;
  std::string status_message_;

  // Header
  HeaderMap headers_;

  //書き込みのバッファ
  utils::ByteVector write_buffer_;

  // File
  // 全てのレスポンスクラスはファイルを返せる必要がある｡
  // なぜならエラー時にファイルを扱う可能性があるからである｡
  int file_fd_;

 public:
  HttpResponse(const config::LocationConf *location, server::Epoll *epoll);
  virtual ~HttpResponse();

  // MakeResponse だけではResponseが作成できない場合に呼ぶメソッド｡
  // 具体的には IsReadyToWrite() と IsAllDataWritingCompleted() が両方 False
  // を返す場合に呼ばれる｡
  // MakeResponse だけでResponseが作成出来る場合にはこの関数は呼ばれないので､
  // 関数には特に何も定義しなくて良い｡
  virtual Result<void> PrepareToWrite(server::ConnSocket *conn_sock);

  void MakeErrorResponse(const HttpStatus status);

  // すべてのデータの write が完了したか
  virtual bool IsAllDataWritingCompleted();

  const std::vector<std::string> &GetHeader(const std::string &header);
  Result<void> WriteToSocket(const int fd);

 protected:
  // ファイルをopenし､Epollで監視する
  Result<void> RegisterFile(const std::string &file_path);
  Result<bool> ReadFile();

  // ========================================================================
  // Getter and Setter
  void SetHttpVersion(const std::string &http_version);
  void SetStatus(HttpStatus status);
  void SetStatus(HttpStatus status, const std::string &status_message);
  void SetStatusMessage(const std::string &status_message);
  void SetHeader(const std::string &header, const std::string &value);
  void AppendHeader(const std::string &header, const std::string &value);

  static bool IsRequestHasConnectionClose(HttpRequest &request);

 private:
  HttpResponse();
  HttpResponse(const HttpResponse &rhs);
  HttpResponse &operator=(const HttpResponse &rhs);

  void LoadRequest(server::ConnSocket *conn_sock);
  // StatusLine と Headers をバイト列にする
  utils::ByteVector SerializeStatusAndHeader() const;
  utils::ByteVector SerializeStatusLine() const;
  utils::ByteVector SerializeHeaders() const;

  Result<WritingPhase> PrepareResponseBody();

  void SerializeResponse(const std::string &body);
  std::string MakeErrorResponseBody(HttpStatus status);

  void MakeAutoIndexResponse(const std::string &abs,
                             const std::string &relative);

  static std::string MakeAutoIndex(const std::string &root_path,
                                   const std::string &relative_path);
};

}  // namespace http

#endif

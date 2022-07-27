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
  // レスポンスの種類
  enum EResponseType { kHttpResponse, kHttpCgiResponse };

  // レスポンスの作成状況
  enum CreateResponsePhase {
    kExecuteRequest,
    kStatusAndHeader,
    kBody,
    kComplete
  };

  // 1回のreadで何バイト読み取るか
  static const unsigned long kBytesPerRead = 1024;  // 1KB

  const config::LocationConf *location_;
  server::Epoll *epoll_;

  // レスポンスの作成状況
  CreateResponsePhase phase_;

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
  EResponseType response_type_;

 public:
  HttpResponse(const config::LocationConf *location, server::Epoll *epoll,
               EResponseType response_type = kHttpResponse);
  HttpResponse(const config::LocationConf *location, server::Epoll *epoll,
               const HttpStatus status,
               EResponseType response_type = kHttpResponse);
  virtual ~HttpResponse();

  //レスポンスの内容を作る関数。
  //適宜write_buffer_につめてWriteできるようにする。
  Result<void> PrepareToWrite(server::ConnSocket *conn_sock);

  HttpResponse::CreateResponsePhase MakeErrorResponse(const HttpStatus status);

  // すべてのデータの write が完了したか
  bool IsAllDataWritingCompleted();

  bool IsWriteBufferEmpty() const;

  bool IsCgiResponse() const;

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

  static bool IsRequestHasConnectionClose(const HttpRequest &request);

 private:
  HttpResponse();
  HttpResponse(const HttpResponse &rhs);
  HttpResponse &operator=(const HttpResponse &rhs);

  // CGIとFILEで処理が異なる部分
  virtual CreateResponsePhase ExecuteRequest(server::ConnSocket *conn_sock);
  virtual Result<CreateResponsePhase> MakeResponseBody();

  CreateResponsePhase ExecuteGetRequest(const http::HttpRequest &request);
  CreateResponsePhase ExecutePostRequest(const server::ConnSocket *conn_sock,
                                         const http::HttpRequest &request);
  CreateResponsePhase ExecuteDeleteRequest(const http::HttpRequest &request);

  std::string CreateResourceUrl(const std::string &local_path,
                                const server::ConnSocket *conn_sock,
                                const http::HttpRequest &request);

  // StatusLine と Headers をバイト列にする
  utils::ByteVector SerializeStatusAndHeader() const;
  utils::ByteVector SerializeStatusLine() const;
  utils::ByteVector SerializeHeaders() const;
  CreateResponsePhase MakeResponse(const std::string &body);

  CreateResponsePhase MakeRedirectResponse();
  std::string SerializeErrorResponseBody(HttpStatus status);

  CreateResponsePhase MakeAutoIndexResponse(const std::string &abs,
                                            const std::string &relative);
  Result<std::string> GetResponsableIndexPagePath();

  static Result<std::string> MakeAutoIndex(const std::string &root_path,
                                           const std::string &relative_path);
};

}  // namespace http

#endif

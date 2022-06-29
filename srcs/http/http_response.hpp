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

namespace http {

struct FileBuffer {
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

using namespace result;

class HttpResponse {
 protected:
  static const std::string kDefaultHttpVersion;

  const config::LocationConf *location_;
  // File Response の場合は file_fd を監視したい
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
  FileBuffer *file_buffer_;
  // レスポンス完了後､Epollの監視を解除するために必要
  server::FdEvent *file_fde_;

 public:
  HttpResponse();
  HttpResponse(const config::LocationConf *location, server::Epoll *epoll);
  HttpResponse(const HttpResponse &rhs);
  ~HttpResponse();

  virtual void MakeResponse(server::ConnSocket *conn_sock);

  bool MakeErrorResponse(const config::LocationConf *location,
                         const HttpRequest &request, HttpStatus status);

  virtual Result<void> Write(int fd);

  // データ書き込みが可能か
  virtual bool IsReadyToWrite();

  // すべてのデータの write が完了したか
  virtual bool IsAllDataWritingCompleted();

 protected:
  // ファイルをopenし､Epollで監視する
  Result<void> RegisterFile(std::string file_path);

  // status-line と header-lines を書き込む｡
  // status_and_headers_bytes_ にデータが無い(初回呼び出し)ときには､
  // データをセットする｡
  //
  // 返り値は今回書き込んだバイト数である｡
  // 0ならば全てのバイト書き込みが完了したことになる｡
  Result<int> WriteStatusAndHeader(int fd);

  bool IsStatusAndHeadersWritingCompleted();

  // ========================================================================
  // Getter and Setter
  void SetHttpVersion(const std::string &http_version);
  void SetStatus(HttpStatus status);
  void SetStatus(HttpStatus status, const std::string &status_message);
  void SetStatusMessage(const std::string &status_message);
  void AppendHeader(const std::string &header, const std::string &value);

 private:
  HttpResponse &operator=(const HttpResponse &rhs);
  void WriteStatusLine(int fd) const;
  void WriteHeaders(int fd) const;
  void WriteBody(int fd) const;

  // StatusLine と Headers をバイト列にする
  utils::ByteVector SerializeStatusAndHeader();
  utils::ByteVector SerializeStatusLine() const;
  utils::ByteVector SerializeHeaders() const;

  std::string MakeErrorResponseBody(HttpStatus status);

  // Making response
  bool MakeFileResponse(const config::LocationConf &location,
                        const HttpRequest &request);

  std::string MakeAutoIndex(const std::string &root_path,
                            const std::string &relative_path) const;
};

}  // namespace http

#endif

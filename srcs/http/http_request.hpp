#ifndef HTTP_HTTP_REQUEST_HPP_
#define HTTP_HTTP_REQUEST_HPP_

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "config/config.hpp"
#include "http/types.hpp"
#include "http_constants.hpp"
#include "http_status.hpp"
#include "result/result.hpp"
#include "utils/ByteVector.hpp"
#include "utils/string.hpp"

namespace http {
using namespace result;

namespace method_strs {
const std::string kGet = "GET";
const std::string kPost = "POST";
const std::string kDelete = "DELETE";
}  // namespace method_strs

struct Chunk {
  enum ChunkStatus {
    kReceived = 200,
    kWaiting = 0,
    kErrorLength = PAYLOAD_TOO_LARGE,
    kErrorBadRequest = BAD_REQUEST
  };
  std::string size_str;
  unsigned long data_size;
};

class HttpRequest {
 private:
  enum ParsingPhase {
    kRequestLine,
    kHeaderField,
    kLoadHeader,
    kBody,
    kParsed,
    kError
  };

  std::string method_;
  std::string path_;
  int minor_version_;
  HeaderMap headers_;
  ParsingPhase phase_;
  HttpStatus parse_status_;
  utils::ByteVector body_;  // HTTP リクエストのボディ
  unsigned long body_size_;
  bool is_chunked_;
  const config::VirtualServerConf *vserver_;
  const config::LocationConf *location_;

  // buffer内の文字列で処理を完了できない時、current_bufferに文字列を保持して処理を中断
  // 次のbufferが来るのを待つ

  // ソケットからはデータを細切れでしか受け取れないので一旦バッファに保管し､行ごとに処理する｡

 public:
  HttpRequest();
  HttpRequest(const HttpRequest &rhs);
  const HttpRequest &operator=(const HttpRequest &rhs);
  ~HttpRequest();

  const std::string &GetMethod() const;
  const std::string &GetPath() const;
  HttpStatus GetParseStatus() const;

  void ParseRequest(utils::ByteVector &buffer, const config::Config &conf,
                    const config::PortType &port);
  bool IsCorrectRequest();
  bool IsCorrectStatus();
  bool IsParsed();

  // ========================================================================
  // Getter and Setter
  Result<const std::vector<std::string> &> GetHeader(std::string header) const;
  const utils::ByteVector &GetBody();

 private:
  ParsingPhase ParseRequestLine(utils::ByteVector &buffer);
  ParsingPhase ParseHeaderField(utils::ByteVector &buffer);
  ParsingPhase LoadHeader(const config::Config &conf,
                          const config::PortType &conn_sock);
  ParsingPhase ParseBody(utils::ByteVector &buffer);
  HttpStatus InterpretMethod(const std::string &method);
  HttpStatus InterpretPath(const std::string &path);
  HttpStatus InterpretVersion(const std::string &version);
  HttpStatus InterpretHeaderField(const std::string &str);
  HttpStatus InterpretContentLength(
      const HeaderMap::mapped_type &length_header);
  HttpStatus InterpretTransferEncoding(
      const HeaderMap::mapped_type &encoding_header);
  ParsingPhase ParsePlainBody(utils::ByteVector &buffer);
  ParsingPhase ParseChunkedBody(utils::ByteVector &buffer);

  HttpStatus DecideBodySize();
  bool LoadVirtualServer(const config::Config &conf,
                         const config::PortType &port);
  bool LoadLocation();
  void PrintRequestInfo();
};

}  // namespace http

#endif

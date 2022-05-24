#ifndef HTTP_HTTP_REQUEST_HPP_
#define HTTP_HTTP_REQUEST_HPP_

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "http_constants.hpp"
#include "http_status.hpp"

namespace http {

namespace method_strs {
const std::string kGet = "GET";
const std::string kPost = "POST";
const std::string kDelete = "DELETE";
};  // namespace method_strs

class HttpRequest {
 public:
  typedef unsigned char Byte;
  typedef std::vector<Byte> ByteVector;

 private:
  std::string method_;
  std::string path_;
  std::map<std::string, std::string> headers_;
  ByteVector body_;  // HTTP リクエストのボディ

  // ソケットからはデータを細切れでしか受け取れないので一旦バッファに保管し､行ごとに処理する｡
  ByteVector buffer_;

  enum RequestPhase { kRequestLine, kHeaderField, kBody, kParsed };

  RequestPhase phase_;

  HttpStatus parse_status_;

  static const ByteVector::size_type reserve_size_ = 2 * 1024;  // 2KB

 public:
  HttpRequest();
  HttpRequest(const HttpRequest &rhs);
  HttpRequest &operator=(const HttpRequest &rhs);
  ~HttpRequest();

  void AppendDataToBuffer(Byte *buf, size_t size);
  void ParseRequest();

 private:
  void ParseRequestLine();
  void ParseHeaderField();
  std::string ExtractFromBuffer(const char *pos);
  const char *FindCrlf();
  void EraseBufferHead(size_t size);
  bool CompareBufferHead(const std::string &str);
};

};  // namespace http

#endif

#ifndef HTTP_HTTP_REQUEST_HPP_
#define HTTP_HTTP_REQUEST_HPP_

#include <algorithm>
#include <cctype>
#include <cstdlib>
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
  enum RequestPhase { kRequestLine, kHeaderField, kBody, kParsed };

  std::string method_;
  std::string path_;
  int minor_version_;
  std::map<std::string, std::vector<std::string> > headers_;
  RequestPhase phase_;
  HttpStatus parse_status_;
  ByteVector body_;  // HTTP リクエストのボディ

  // ソケットからはデータを細切れでしか受け取れないので一旦バッファに保管し､行ごとに処理する｡
  ByteVector buffer_;

  static const ByteVector::size_type reserve_size_ = 2 * 1024;  // 2KB

 public:
  HttpRequest();
  HttpRequest(const HttpRequest &rhs);
  HttpRequest &operator=(const HttpRequest &rhs);
  ~HttpRequest();

  void AppendDataToBuffer(Byte *buf, size_t size);
  void ParseRequest();
  bool IsCorrectRequest();

 private:
  void ParseRequestLine();
  void ParseHeaderField();
  void ParseBody();
  HttpStatus InterpretMethod(std::string &str);
  HttpStatus InterpretPath(std::string &str);
  HttpStatus InterpretVersion(std::string &str);
  HttpStatus InterpretHeaderField(std::string &str);

  std::string TrimWhiteSpace(std::string &str);
  const char *FindCrlf();
  std::string ExtractFromBuffer(const char *pos);
  void EraseBufferHead(size_t size);
  bool CompareBufferHead(const std::string &str);
  bool TryExtractBeforeWhiteSpace(std::string &src, std::string &dest);
  bool IsCorrectHTTPVersion(const std::string &str);
  void PrintRequestInfo();
};

};  // namespace http

#endif

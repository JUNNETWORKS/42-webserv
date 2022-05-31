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
#include "utils/ByteVector.hpp"
#include "utils/string.hpp"

namespace http {

namespace method_strs {
const std::string kGet = "GET";
const std::string kPost = "POST";
const std::string kDelete = "DELETE";
};  // namespace method_strs

class HttpRequest {
 private:
  typedef std::map<std::string, std::vector<std::string> > HeaderMap;
  enum ParsingPhase {
    kRequestLine,
    kHeaderField,
    kBodySize,
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

  // ソケットからはデータを細切れでしか受け取れないので一旦バッファに保管し､行ごとに処理する｡

 public:
  HttpRequest();
  HttpRequest(const HttpRequest &rhs);
  HttpRequest &operator=(const HttpRequest &rhs);
  ~HttpRequest();

  void ParseRequest();
  bool IsCorrectRequest();

  utils::ByteVector buffer_;  // bufferはSocketInfoに移動予定

 private:
  ParsingPhase ParseRequestLine();
  ParsingPhase ParseHeaderField();
  ParsingPhase ParseBodySize();
  ParsingPhase ParseBody();
  HttpStatus InterpretMethod(std::string &str);
  HttpStatus InterpretPath(std::string &str);
  HttpStatus InterpretVersion(std::string &str);
  HttpStatus InterpretHeaderField(std::string &str);
  HttpStatus InterpretContentLength(
      const HeaderMap::mapped_type &length_header);

  HttpStatus DecideBodySize();
  void PrintRequestInfo();
};

};  // namespace http

#endif

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
  enum ParsePhase { kRequestLine, kHeaderField, kBody, kParsed };

  std::string method_;
  std::string path_;
  int minor_version_;
  std::map<std::string, std::vector<std::string> > headers_;
  ParsePhase phase_;
  HttpStatus parse_status_;
  utils::ByteVector body_;  // HTTP リクエストのボディ

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
  ParsePhase ParseRequestLine();
  ParsePhase ParseHeaderField();
  ParsePhase ParseBody();
  HttpStatus InterpretMethod(std::string &str);
  HttpStatus InterpretPath(std::string &str);
  HttpStatus InterpretVersion(std::string &str);
  HttpStatus InterpretHeaderField(std::string &str);

  void PrintRequestInfo();
};

};  // namespace http

#endif

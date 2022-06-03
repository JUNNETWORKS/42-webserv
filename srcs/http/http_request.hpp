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
}  // namespace method_strs

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

  // buffer内の文字列で処理を完了できない時、current_bufferに文字列を保持して処理を中断
  // 次のbufferが来るのを待つ
  utils::ByteVector current_buffer_;

  // ソケットからはデータを細切れでしか受け取れないので一旦バッファに保管し､行ごとに処理する｡

 public:
  HttpRequest();
  HttpRequest(const HttpRequest &rhs);
  HttpRequest &operator=(const HttpRequest &rhs);
  ~HttpRequest();

  const std::string &GetPath() const;

  void ParseRequest(utils::ByteVector &buffer);
  bool IsCorrectRequest();
  bool IsCorrectStatus();
  bool IsParsed();

 private:
  void SaveCurrentBuffer(utils::ByteVector &buffer);
  void LoadCurrentBuffer(utils::ByteVector &buffer);
  ParsingPhase ParseRequestLine(utils::ByteVector &buffer);
  ParsingPhase ParseHeaderField(utils::ByteVector &buffer);
  ParsingPhase ParseBodySize();
  ParsingPhase ParseBody(utils::ByteVector &buffer);
  HttpStatus InterpretMethod(std::string &str);
  HttpStatus InterpretPath(std::string &str);
  HttpStatus InterpretVersion(std::string &str);
  HttpStatus InterpretHeaderField(std::string &str);
  HttpStatus InterpretContentLength(
      const HeaderMap::mapped_type &length_header);

  HttpStatus DecideBodySize();
  void PrintRequestInfo();
};

}  // namespace http

#endif

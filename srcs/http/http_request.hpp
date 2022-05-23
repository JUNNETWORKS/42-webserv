#ifndef HTTP_HTTP_REQUEST_HPP_
#define HTTP_HTTP_REQUEST_HPP_

#include <map>
#include <string>
#include <vector>

namespace http {

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

  static const ByteVector::size_type reserve_size_ = 2 * 1024;  // 2KB

 public:
  HttpRequest();
  HttpRequest(const HttpRequest &rhs);
  HttpRequest &operator=(const HttpRequest &rhs);
  ~HttpRequest();

  void AppendDataToBuffer(Byte *buf, size_t size);
  void ParseOneLine();
};

};  // namespace http

#endif

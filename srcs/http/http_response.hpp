#ifndef HTTP_HTTP_RESPONSE_HPP_
#define HTTP_HTTP_RESPONSE_HPP_

#include <map>
#include <string>
#include <vector>

#include "http/http_status.hpp"

namespace http {

class HttpResponse {
 private:
  typedef unsigned char Byte;
  typedef std::vector<Byte> ByteVector;

  HttpStatus status_;
  std::map<std::string, std::string> headers_;
  ByteVector body_;

  // レスポンスとして返すバイトデータをすべて格納しておく｡
  ByteVector buffer_;
  Byte *buf_position_;  // buffer_ の内部配列へのポインタ｡次writeを開始する位置｡

  static const ByteVector::size_type reserve_size_ = 2 * 1024;  // 2KB

 public:
  HttpResponse();
  HttpResponse(const HttpResponse &rhs);
  HttpResponse &operator=(const HttpResponse &rhs);
  ~HttpResponse();
};

};  // namespace http

#endif

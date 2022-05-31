#ifndef HTTP_HTTP_RESPONSE_HPP_
#define HTTP_HTTP_RESPONSE_HPP_

#include <map>
#include <string>
#include <vector>

#include "http/http_status.hpp"

namespace http {

class HttpResponse {
 private:
  HttpStatus status_;
  std::map<std::string, std::string> headers_;
  std::string status_line_;
  std::string header_;
  std::string body_;

 public:
  HttpResponse();

  HttpResponse(const HttpResponse &rhs);

  HttpResponse &operator=(const HttpResponse &rhs);

  ~HttpResponse();

  void SetStatusLine(std::string status_line);
  void SetHeader(std::string header);
  void SetBody(std::string body);

  const std::string &GetStatusLine() const;
  const std::string &GetHeader() const;
  const std::string &GetBody() const;

  void Write(int fd) const;
  bool LoadFile(const std::string &path);
};

};  // namespace http

#endif

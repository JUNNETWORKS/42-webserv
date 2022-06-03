#ifndef HTTP_HTTP_RESPONSE_HPP_
#define HTTP_HTTP_RESPONSE_HPP_

#include <map>
#include <string>
#include <vector>

#include "http/http_status.hpp"

namespace http {

class HttpResponse {
 private:
  typedef std::map<std::string, std::vector<std::string> > HeadersType;

  HttpStatus status_;
  HeadersType headers_;
  std::string status_line_;
  std::string body_;

 public:
  HttpResponse();

  HttpResponse(const HttpResponse &rhs);

  HttpResponse &operator=(const HttpResponse &rhs);

  ~HttpResponse();

  void SetStatusLine(const std::string &status_line);
  void AppendHeader(const std::string &header, const std::string &value);
  void SetBody(const std::string &body);

  const std::string &GetStatusLine() const;
  const std::string &GetBody() const;

  void Write(int fd) const;
  bool LoadFile(const std::string &path);

 private:
  void WriteStatusLine(int fd) const;
  void WriteHeaders(int fd) const;
  void WriteBody(int fd) const;
};

}  // namespace http

#endif

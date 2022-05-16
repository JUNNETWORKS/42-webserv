#ifndef HTTP_HPP_
#define HTTP_HPP_

#include <map>

namespace http {

namespace methods {
const std::string GET = "GET";
const std::string POST = "POST";
const std::string DELETE = "DELETE";
};  // namespace methods

class HttpRequest {
 private:
  std::map<std::string, std::string> headers;
};

class HttpResponse {};

};  // namespace http

#endif

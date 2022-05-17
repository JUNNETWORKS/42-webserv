#ifndef HTTP_HTTP_REQUEST_HPP_
#define HTTP_HTTP_REQUEST_HPP_

#include <map>

namespace http {

namespace method_strs {
const std::string kGet = "GET";
const std::string kPost = "POST";
const std::string kDelete = "DELETE";
};  // namespace method_strs

class HttpRequest {
 private:
  std::map<std::string, std::string> headers;
};

};  // namespace http

#endif

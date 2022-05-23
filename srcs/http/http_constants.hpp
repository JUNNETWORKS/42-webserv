#ifndef HTTP_CONSTANTS_HPP_
#define HTTP_CONSTANTS_HPP_

#include <string>

namespace http {
const std::string kCrlf = "\r\n";
const std::string kHeaderBoundary = kCrlf + kCrlf;
}  // namespace http

#endif
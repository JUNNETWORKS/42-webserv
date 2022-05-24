#ifndef HTTP_CONSTANTS_HPP_
#define HTTP_CONSTANTS_HPP_

#include <string>

namespace http {
const std::string kCrlf = "\r\n";
const std::string kHeaderBoundary = kCrlf + kCrlf;
const std::string kHttpVersionPrefix = "HTTP/";
const std::string kExpectMinorVersion = "1.";
const int kMinorVersionDigitLimit = 3;
}  // namespace http

#endif
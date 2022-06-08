#ifndef HTTP_CONSTANTS_HPP_
#define HTTP_CONSTANTS_HPP_

#include <string>

namespace http {
const std::string kCrlf = "\r\n";
const std::string kHeaderBoundary = kCrlf + kCrlf;
const std::string kHttpVersionPrefix = "HTTP/";
const std::string kExpectMajorVersion = "1.";
const int kMinorVersionDigitLimit = 3;
const std::string kOWS = "\t ";
const std::string kNotAlnumTchars = "!#$%&'*+-.^_`|~";
}  // namespace http

#endif

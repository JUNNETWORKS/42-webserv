#ifndef HTTP_HTTP_STATUS_HPP
#define HTTP_HTTP_STATUS_HPP

#include <map>
#include <string>

namespace http {

enum HttpStatus {
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NO_CONTENT = 204,
  MULTIPLE_CHOICES = 300,
  MOVED_PERMANENTLY = 301,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  TEMPORARY_REDIRECT = 307,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  REQUEST_TIMEOUT = 408,
  PAYLOAD_TOO_LARGE = 413,
  URI_TOO_LONG = 414,
  SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  SERVICE_UNAVAILABLE = 503,
  HTTP_VERSION_NOT_SUPPORTED = 505
};

class StatusCodes {
 private:
  static const std::map<unsigned long, std::string> status_messages_;

 public:
  static bool IsHttpStatus(unsigned long status);
  static std::string GetMessage(unsigned long status);

 private:
  StatusCodes();
  StatusCodes(const StatusCodes &rhs);
  StatusCodes &operator=(const StatusCodes &rhs);
  ~StatusCodes();

  // https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
  static std::map<unsigned long, std::string> CreateStatusMessages();
};

}  // namespace http

#endif

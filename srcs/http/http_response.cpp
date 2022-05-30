#include "http/http_response.hpp"

#include <algorithm>
#include <vector>

#include "utils/io.hpp"
#include "utils/string.hpp"

namespace http {

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse &rhs) {
  *this = rhs;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &rhs) {
  if (this != &rhs) {
    status_ = rhs.status_;
    headers_ = rhs.headers_;
    status_line_ = rhs.status_line_;
    header_ = rhs.header_;
    body_ = rhs.body_;
    // buffer_ = rhs.buffer_;
    // buf_position_ = buffer_.data() + (rhs.buf_position_ -
    // rhs.buffer_.data());
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

//========================================================================
// setter, getter

void HttpResponse::setStatusLine(std::string status_line) {
  status_line_ = status_line;
}
void HttpResponse::setHeader(std::string header) {
  header_ = header;
}
void HttpResponse::setBody(std::string body) {
  body_ = body;
}

const std::string &HttpResponse::getStatusLine() const {
  return status_line_;
}
const std::string &HttpResponse::getHeader() const {
  return header_;
}
const std::string &HttpResponse::getBody() const {
  return body_;
}

void HttpResponse::write(int fd) const {
  utils::ft_putstr_fd(status_line_, fd);
  utils::ft_putstr_fd("\r\n", fd);
  utils::ft_putstr_fd(header_, fd);
  utils::ft_putstr_fd("\r\n", fd);
  utils::ft_putstr_fd("\r\n", fd);
  utils::ft_putstr_fd(body_, fd);
  utils::ft_putstr_fd("\r\n", fd);
}

//========================================================================
//

static std::string makeAutoIndex(const std::string &path) {
  std::string html;
  std::vector<std::string> file_vec;
  std::string head = "<html>\n<head><title>Index of " + path +
                     "/"
                     "</title></head>\n"
                     "<body bgcolor=\"black\">\n"
                     "<h1>Index of " +
                     path + "</h1><hr><pre>";
  std::string tail =
      "</pre><hr></body>\n"
      "</html>\n";

  utils::getFileList(path, file_vec);
  std::sort(file_vec.begin(), file_vec.end());
  std::string is_dir;
  for (std::size_t i = 0; i < file_vec.size(); i++) {
    if (utils::isDir(path + "/" + file_vec[i])) {
      is_dir = "/";
    } else {
      is_dir = "";
    }
    html += "<a href=\"" + file_vec[i] + is_dir + "\">";
    html += file_vec[i] + is_dir + "</a>\n";
  }

  return head + html + tail;
}

bool HttpResponse::loadfile(const std::string &file_path) {
  std::string file_data;

  if (!utils::isFileExist(file_path)) {
    // status_ = NOT_FOUND;
    return false;
  }

  if (utils::isDir(file_path)) {
    body_ = makeAutoIndex(file_path);
    setHeader("Content-Type: text/html");
    return true;
  }

  if (!utils::readFile(file_path, file_data)) {
    // status_ = FORBIDDEN;  // TODO
    return false;
  }

  setBody(file_data);
  setHeader("Content-Type: text/plain");
  return true;
}

};  // namespace http

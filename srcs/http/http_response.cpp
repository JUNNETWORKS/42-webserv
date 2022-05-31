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
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

//========================================================================
// setter, getter

void HttpResponse::SetStatusLine(std::string status_line) {
  status_line_ = status_line;
}
void HttpResponse::SetHeader(std::string header) {
  header_ = header;
}
void HttpResponse::SetBody(std::string body) {
  body_ = body;
}

const std::string &HttpResponse::GetStatusLine() const {
  return status_line_;
}
const std::string &HttpResponse::GetHeader() const {
  return header_;
}
const std::string &HttpResponse::GetBody() const {
  return body_;
}

void HttpResponse::Write(int fd) const {
  utils::PutStrFd(status_line_, fd);
  utils::PutStrFd("\r\n", fd);
  utils::PutStrFd(header_, fd);
  utils::PutStrFd("\r\n", fd);
  utils::PutStrFd("\r\n", fd);
  utils::PutStrFd(body_, fd);
  utils::PutStrFd("\r\n", fd);
}

//========================================================================
//

static std::string MakeAutoIndex(const std::string &path) {
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

  utils::GetFileList(path, file_vec);
  std::sort(file_vec.begin(), file_vec.end());
  std::string is_dir;
  for (std::size_t i = 0; i < file_vec.size(); i++) {
    if (utils::IsDir(path + "/" + file_vec[i])) {
      is_dir = "/";
    } else {
      is_dir = "";
    }
    html += "<a href=\"" + file_vec[i] + is_dir + "\">";
    html += file_vec[i] + is_dir + "</a>\n";
  }

  return head + html + tail;
}

bool HttpResponse::LoadFile(const std::string &file_path) {
  std::string file_data;

  if (!utils::IsFileExist(file_path)) {
    // status_ = NOT_FOUND;
    return false;
  }

  if (utils::IsDir(file_path)) {
    body_ = MakeAutoIndex(file_path);
    SetHeader("Content-Type: text/html");
    return true;
  }

  if (!utils::ReadFile(file_path, file_data)) {
    // status_ = FORBIDDEN;  // TODO
    return false;
  }

  SetBody(file_data);
  SetHeader("Content-Type: text/plain");
  return true;
}

};  // namespace http

#include "http/http_response.hpp"

#include <algorithm>
#include <vector>

#include "http/http_constants.hpp"
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
    body_ = rhs.body_;
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::AppendHeader(const std::string &header,
                                const std::string &value) {
  headers_[header].push_back(value);
}

//========================================================================
// setter, getter

void HttpResponse::SetStatusLine(const std::string &status_line) {
  status_line_ = status_line;
}
void HttpResponse::SetBody(const std::string &body) {
  body_ = body;
}

const std::string &HttpResponse::GetStatusLine() const {
  return status_line_;
}
const std::string &HttpResponse::GetBody() const {
  return body_;
}

void HttpResponse::Write(int fd) const {
  WriteStatusLine(fd);
  WriteHeaders(fd);
  WriteBody(fd);
}

void HttpResponse::WriteStatusLine(int fd) const {
  utils::PutStrFd(status_line_, fd);
  utils::PutStrFd(http::kCrlf, fd);
}

void HttpResponse::WriteHeaders(int fd) const {
  for (HeaderMap::const_iterator headers_it = headers_.begin();
       headers_it != headers_.end(); ++headers_it) {
    typedef HeaderMap::mapped_type HeaderValuesType;
    HeaderValuesType header_values = headers_it->second;
    for (HeaderValuesType::const_iterator value_it = header_values.begin();
         value_it != header_values.end(); ++value_it) {
      const std::string header = headers_it->first + ": " + *value_it;
      utils::PutStrFd(header, fd);
      utils::PutStrFd(http::kCrlf, fd);
    }
  }
  utils::PutStrFd(http::kCrlf, fd);
}

void HttpResponse::WriteBody(int fd) const {
  utils::PutStrFd(body_, fd);
  utils::PutStrFd(http::kCrlf, fd);
}

//========================================================================
//

static std::string MakeAutoIndex(const std::string &path) {
  std::string html;
  std::vector<std::string> file_vec;
  std::string head = "<html>\n<head><title>Index of " + path +
                     "/"
                     "</title></head>\n"
                     "<body bgcolor=\"white\">\n"
                     "<h1>Index of " +
                     path + "</h1><hr><pre>";
  std::string tail =
      "</pre><hr></body>\n"
      "</html>\n";

  utils::GetFileList(path, file_vec);
  std::sort(file_vec.begin(), file_vec.end());
  std::string is_dir;
  for (size_t i = 0; i < file_vec.size(); i++) {
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
    // status_ = NOT_FOUND;  // TODO : レスポンスステータスコードを設定する
    return false;
  }

  if (utils::IsDir(file_path)) {
    body_ = MakeAutoIndex(file_path);
    AppendHeader("Content-Type", "text/html");
    return true;
  }

  if (!utils::ReadFile(file_path, file_data)) {
    // status_ = FORBIDDEN;  // TODO : レスポンスステータスコードを設定する
    return false;
  }

  SetBody(file_data);
  AppendHeader("Content-Type", "text/plain");
  return true;
}

}  // namespace http

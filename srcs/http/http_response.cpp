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
    header_ = rhs.header_;
    body_ = rhs.body_;
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

//========================================================================
// setter, getter

void HttpResponse::SetStatusLine(const std::string &status_line) {
  status_line_ = status_line;
}
void HttpResponse::SetHeader(const std::string &header) {
  header_ = header;
}
void HttpResponse::SetBody(const std::string &body) {
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
  utils::PutStrFd(http::kCrlf, fd);
  utils::PutStrFd(header_, fd);
  utils::PutStrFd(http::kHeaderBoundary, fd);
  utils::PutStrFd(body_, fd);
  utils::PutStrFd(http::kCrlf, fd);
}

//========================================================================
//

// TODO : configから取得するようにする
static const std::string GetRootDir() {
  return "/public";
}

static std::string MakeAutoIndex(const std::string &path) {
  std::string html;
  std::vector<std::string> file_vec;
  std::string head = "<html>\n<head><title>Index of " + path +
                     "</title></head>\n"
                     "<body bgcolor=\"white\">\n"
                     "<h1>Index of " +
                     path + "</h1><hr><pre>";
  std::string tail =
      "</pre><hr></body>\n"
      "</html>\n";

  // TODO : / が連続するパターンの考慮
  utils::GetFileList(GetRootDir() + path, file_vec);
  std::sort(file_vec.begin(), file_vec.end());
  std::string is_dir;
  for (size_t i = 0; i < file_vec.size(); i++) {
    if (utils::IsDir(GetRootDir() + path + "/" + file_vec[i])) {
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

  if (!utils::IsFileExist(GetRootDir() + file_path)) {
    // status_ = NOT_FOUND;  // TODO : レスポンスステータスコードを設定する
    return false;
  }

  if (utils::IsDir(GetRootDir() + file_path)) {
    body_ = MakeAutoIndex(file_path);
    SetHeader("Content-Type: text/html");
    return true;
  }

  if (!utils::ReadFile(GetRootDir() + file_path, file_data)) {
    // status_ = FORBIDDEN;  // TODO : レスポンスステータスコードを設定する
    return false;
  }

  SetBody(file_data);
  SetHeader("Content-Type: text/plain");
  return true;
}

}  // namespace http

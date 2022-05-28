#include "config/config_parser.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstring>
#include <string>

#include "config/config.hpp"
#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"
#include "http/constants.hpp"
#include "utils/string.hpp"

namespace config {

namespace {

int GetFileSize(const std::string &filename) {
  struct stat sbuf;
  if (stat(filename.c_str(), &sbuf) < 0) {
    return -1;
  }
  return sbuf.st_size;
};

}  // namespace

Parser::Parser() : file_content_(), buf_idx_(0) {}

Parser::Parser(const Parser &rhs) {
  *this = rhs;
}

Parser &Parser::operator=(const Parser &rhs) {
  if (&rhs != this) {
    file_content_ = rhs.file_content_;
    buf_idx_ = buf_idx_;
  }
  return *this;
}

Parser::~Parser() {}

void Parser::LoadFile(const std::string &filename) {
  int filesize = GetFileSize(filename);
  if (filesize < 0) {
    throw ParserException("Failed to get file size in LoadFile().");
  }
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    throw ParserException("Failed open() in LoadFile().");
  }
  void *p = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
  if (p == NULL) {
    throw ParserException("Failed mmap() in LoadFile().");
  }
  file_content_ = std::string(static_cast<char *>(p));
  munmap(p, filesize);
}

void Parser::LoadData(const std::string &data) {
  file_content_ = data;
}

Config Parser::ParseConfig() {
  Config config;
  while (!IsReachedEOF()) {
    SkipSpaces();
    std::string directive = GetWord();
    if (directive == "server") {
      ParseServerBlock(config);
    } else {
      throw ParserException("Unknown directive in config.");
    }
    SkipSpaces();
  }
  return config;
}

void Parser::ParseServerBlock(Config &config) {
  VirtualServerConf vserver;
  SkipSpaces();
  if (GetC() != '{') {
    throw ParserException("There was no '{' after the Server directive.");
  }
  while (GetC() != '}') {
    UngetC();
    SkipSpaces();
    std::string directive = GetWord();
    if (directive == "listen") {
      ParseListenDirective(vserver);
    } else if (directive == "server_name") {
      ParseServerNameDirective(vserver);
    } else if (directive == "location") {
      ParseLocationBlock(vserver, false);
    } else if (directive == "location_back") {
      ParseLocationBlock(vserver, true);
    } else {
      throw ParserException("Unknown directive in server block.");
    }
    SkipSpaces();
  }

  config.AppendVirtualServerConf(vserver);
}

void Parser::ParseListenDirective(VirtualServerConf &vserver) {
  if (!vserver.GetListenPort().empty()) {
    throw ParserException("Port is empty.");
  }
  SkipSpaces();
  std::string port = GetWord();
  SkipSpaces();
  if (!IsValidPort(port) || GetC() != ';') {
    throw ParserException("Port directive's argument is invalid.");
  }
  vserver.SetListenPort(port);
}

void Parser::ParseServerNameDirective(VirtualServerConf &vserver) {
  SkipSpaces();
  while (!IsReachedEOF() && GetC() != ';') {
    UngetC();
    std::string domain_name = GetWord();
    if (!IsDomainName(domain_name)) {
      throw ParserException("server_name is invalid.");
    }
    vserver.AppendServerName(domain_name);
    SkipSpaces();
  }
}

void Parser::ParseLocationBlock(VirtualServerConf &vserver,
                                bool is_location_back) {
  LocationConf location;
  location.SetIsBackwardSearch(is_location_back);
  SkipSpaces();
  // Get path
  std::string path_pattern = GetWord();
  location.SetPathPattern(path_pattern);
  SkipSpaces();
  if (GetC() != '{') {
    throw ParserException("There was no '{' after the Location directive.");
  }
  while (GetC() != '}') {
    UngetC();
    SkipSpaces();
    std::string directive = GetWord();
    if (directive == "allow_method") {
      ParseAllowMethodDirective(location);
    } else if (directive == "client_max_body_size") {
      ParseClientMaxBodySizeDirective(location);
    } else if (directive == "root") {
      ParseRootDirective(location);
    } else if (directive == "index") {
      ParseIndexDirective(location);
    } else if (directive == "autoindex") {
      ParseAutoindexDirective(location);
    } else if (directive == "is_cgi") {
      ParseIscgiDirective(location);
    } else if (directive == "error_page") {
      ParseErrorPage(location);
    } else if (directive == "return") {
      ParseReturnDirective(location);
    } else {
      throw ParserException("Unknown directive in Location block.");
    }
    SkipSpaces();
  }

  vserver.AppendLocation(location);
}

void Parser::ParseAllowMethodDirective(LocationConf &location) {
  SkipSpaces();
  while (GetC() != ';') {
    UngetC();
    SkipSpaces();
    std::string method = GetWord();
    if (!IsHttpMethod(method)) {
      throw ParserException("Invalid method.");
    }
    location.AppendAllowedMethod(method);
  }
}

void Parser::ParseClientMaxBodySizeDirective(LocationConf &location) {
  SkipSpaces();
  std::string body_size_str = GetWord();
  int64_t body_size;
  try {
    body_size = utils::Stoll(body_size_str);
  } catch (const std::exception &e) {
    throw ParserException("Invalid body size.");
  }
  if (body_size < 0) {
    throw ParserException("Invalid body size.");
  }
  location.SetClientMaxBodySize(body_size);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException(
        "Can't find semicolon after client_max_body_size directive.");
  }
}

void Parser::ParseRootDirective(LocationConf &location) {
  SkipSpaces();
  std::string root = GetWord();
  location.SetRootDir(root);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after root directive.");
  }
}

void Parser::ParseIndexDirective(LocationConf &location) {
  SkipSpaces();
  while (!IsReachedEOF() && GetC() != ';') {
    UngetC();
    std::string filename = GetWord();
    location.AppendIndexPages(filename);
    SkipSpaces();
  }
}

void Parser::ParseErrorPage(LocationConf &location) {
  SkipSpaces();
  // 最後のargがエラーページのpathになってる｡
  std::vector<std::string> args;
  while (GetC() != ';') {
    UngetC();
    SkipSpaces();
    std::string arg = GetWord();
    args.push_back(arg);
  }
  UngetC();

  if (args.size() < 2) {
    throw ParserException(
        "error_page need to specify http status and error page path.");
  }

  std::string &error_page = args.back();
  for (size_t i = 0; i < args.size() - 1; ++i) {
    if (!IsValidHttpStatusCode(args[i])) {
      // TODO: HTTPステータスコードの範囲内かチェックするメソッドで検査する｡
      throw ParserException("error_page directive arg isn't valid number.");
    }
    http::HttpStatus status =
        static_cast<http::HttpStatus>(atoi(args[i].c_str()));
    if (location.GetErrorPages().find(status) ==
        location.GetErrorPages().end()) {
      location.AppendErrorPages(status, error_page);
    }
  }

  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after error_page directive.");
  }
}

void Parser::ParseAutoindexDirective(LocationConf &location) {
  SkipSpaces();
  std::string on_or_off = GetWord();
  bool is_autoindex_enabled = ParseOnOff(on_or_off);
  location.SetAutoIndex(is_autoindex_enabled);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after autoindex directive.");
  }
}

void Parser::ParseIscgiDirective(LocationConf &location) {
  SkipSpaces();
  std::string on_or_off = GetWord();
  bool is_cgi = ParseOnOff(on_or_off);
  location.SetAutoIndex(is_cgi);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after is_cgi directive.");
  }
}

void Parser::ParseReturnDirective(LocationConf &location) {
  SkipSpaces();
  std::string url = GetWord();
  location.SetRedirectUrl(url);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after return directive.");
  }
}

// Parser utils

void Parser::SkipSpaces() {
  if (IsReachedEOF()) {
    return;
  }
  if (!isspace(GetC())) {
    UngetC();
    return;
  }
  while (!IsReachedEOF() && isspace(GetC())) {
  }
  if (!IsReachedEOF()) {
    UngetC();
  }
}

char Parser::GetC() {
  if (buf_idx_ >= file_content_.length()) {
    throw ParserException("GetC: buf_idx_ has already reached to EOF.");
  }
  return file_content_[buf_idx_++];
}

char Parser::UngetC() {
  if (buf_idx_ == 0) {
    throw ParserException("UngetC");
  }
  buf_idx_--;
  return file_content_[buf_idx_];
}

std::string Parser::GetWord() {
  std::string word;

  bool is_escaped_char = false;
  char c = GetC();
  // 継続条件
  // EOFに到達してない
  // 空白文字じゃないし､ディレクティブ終了の';'でもない
  // 空白文字､';' だとしてもエスケープされている
  while (!IsReachedEOF() &&
         ((!isspace(c) && !strchr(";{}", c)) ||
          (is_escaped_char && (isspace(c) || strchr(";{}", c))))) {
    if (!is_escaped_char && c == '\\') {
      is_escaped_char = true;
    } else {
      word += c;
      is_escaped_char = false;
    }
    c = GetC();
  }
  UngetC();
  return word;
}

bool Parser::IsUnsignedNumber(const std::string &str) {
  if (str.empty()) {
    return false;
  }
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (*it < '0' || *it > '9') {
      return false;
    }
  }
  return true;
}

bool Parser::IsDomainName(const std::string &domain_name) {
  if (domain_name.empty() || domain_name.size() > kMaxDomainLength) {
    return false;
  }

  // split to labels
  std::vector<std::string> labels = utils::SplitString(domain_name, ".");

  // check each label
  for (std::vector<std::string>::const_iterator it = labels.begin();
       it != labels.end(); ++it) {
    if (!IsDomainLabel(*it)) {
      return false;
    }
  }
  return true;
}

bool Parser::IsDomainLabel(const std::string &label) {
  if (label.empty() || label.length() > kMaxDomainLabelLength) {
    return false;
  }
  size_t idx = 0;
  if (!isalnum(label[idx])) {
    return false;
  }
  while (idx < label.length() - 1) {
    if (!isalnum(label[idx]) && label[idx] != '-') {
      return false;
    }
    idx++;
  }
  if (!isalnum(label[idx])) {
    return false;
  }
  return true;
}

bool Parser::IsHttpMethod(const std::string &method) {
  if (method == http::kGet || method == http::kPost ||
      method == http::kDelete) {
    return true;
  }
  return false;
}

bool Parser::IsValidHttpStatusCode(const std::string &code) {
  // TODO: HTTP Status Code が3桁かどうか以外の条件もありそう｡
  if (IsUnsignedNumber(code) && code.length() == 3) {
    return true;
  }
  return false;
}

bool Parser::IsValidPort(const std::string port) {
  if (!IsUnsignedNumber(port)) {
    return false;
  }

  long long num;
  try {
    num = utils::Stoll(port);
  } catch (...) {
    return false;
  }

  return num >= kMinPortNumber && num <= kMaxPortNumber;
}

bool Parser::ParseOnOff(const std::string &on_or_off) {
  if (on_or_off == "on") {
    return true;
  } else if (on_or_off == "off") {
    return false;
  }
  throw ParserException("ParseOnOff");
}

bool Parser::IsReachedEOF() {
  return buf_idx_ >= file_content_.length();
}

Parser::ParserException::ParserException(const char *errmsg)
    : errmsg_(errmsg) {}

const char *Parser::ParserException::what() const throw() {
  return errmsg_;
}

};  // namespace config

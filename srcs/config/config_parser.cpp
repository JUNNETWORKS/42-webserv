#include "config/config_parser.hpp"

#include <fcntl.h>
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

Parser::Parser(const std::string &filename) : file_content_(), buf_idx_(0) {
  LoadFileData(filename);
}

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

void Parser::LoadFileData(const std::string &filename) {
  int filesize = GetFileSize(filename);
  if (filesize < 0) {
    throw ParserException("Failed to get file size in LoadFileData().");
  }
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    throw ParserException("Failed open() in LoadFileData().");
  }
  void *p = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
  if (p == NULL) {
    throw ParserException("Failed mmap() in LoadFileData().");
  }
  file_content_ = std::string(static_cast<char *>(p));
  munmap(p, filesize);
}

Config Parser::ParseConfig() {
  Config config;
  while (!IsReachedEOF()) {
    SkipSpaces();
    std::string directive = GetWord();
    if (directive == "server") {
      ParseServerBlock(config);
    } else {
      throw ParserException();
    }
  }
  return config;
}

void Parser::ParseServerBlock(Config &config) {
  VirtualServerConf vserver;
  SkipSpaces();
  if (GetC() != '{') {
    throw ParserException();
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
      // TODO: location_back対応
      ParseLocationBlock(vserver);
    } else {
      throw ParserException("Unknown directive in server block.");
    }
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
  // TODO: GetC() != ';' の記述を1つの関数にまとめる
  if (!IsUnsignedNumber(port) || GetC() != ';') {
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

void Parser::ParseLocationBlock(VirtualServerConf &vserver) {
  LocationConf location;
  SkipSpaces();
  if (GetC() != '{') {
    throw ParserException();
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
    } else if (directive == "return") {
      ParseReturnDirective(location);
    } else {
      throw ParserException("Unknown directive in server block.");
    }
  }

  vserver.AppendLocation(location);
}

void Parser::ParseAllowMethodDirective(LocationConf &location) {
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
  if (location.GetClientMaxBodySizeKB() >= 0) {
    throw ParserException("client_max_body_size has already set.");
  }
  SkipSpaces();
  std::string body_size_str = GetWord();
  int64_t body_size;
  try {
    body_size = utils::stoll(body_size_str);
  } catch (const std::exception &e) {
    throw ParserException("Invalid body size.");
  }
  if (body_size < 0) {
    throw ParserException("Invalid body size.");
  }
  location.SetClientMaxBodySizeKB(body_size);
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
  if (!IsReachedEOF() && !isspace(GetC())) {
    UngetC();
    return;
  }
  while (isspace(GetC())) {
  }
  UngetC();
}

char Parser::GetC() {
  if (buf_idx_ >= file_content_.length() - 1) {
    throw ParserException("GetC");
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

// TODO: ';' がファイル名に含まれている場合などに正しく動かない
std::string Parser::GetWord() {
  std::string word;

  char c = GetC();
  while (!IsReachedEOF() && (!isspace(c))) {
    word += c;
    c = GetC();
  }
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
  if (domain_name.empty()) {
    return false;
  }
  size_t idx = 0;
  if (!isalnum(domain_name[idx])) {
    return false;
  }
  while (idx < domain_name.length() - 1) {
    if (!isalnum(domain_name[idx]) && domain_name[idx] != '-') {
      return false;
    }
    idx++;
  }
  if (!isalnum(domain_name[idx])) {
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

bool Parser::ParseOnOff(const std::string &on_or_off) {
  if (on_or_off == "yes") {
    return true;
  } else if (on_or_off == "no") {
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

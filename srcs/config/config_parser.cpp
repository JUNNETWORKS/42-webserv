#include "config/config_parser.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>

#include <cstdarg>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "config/config.hpp"
#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"
#include "http/http_request.hpp"
#include "http/http_status.hpp"
#include "utils/string.hpp"

namespace config {

Parser::Parser() : file_content_(), buf_idx_(0) {}

Parser::Parser(const Parser &rhs) {
  *this = rhs;
}

Parser &Parser::operator=(const Parser &rhs) {
  if (&rhs != this) {
    file_content_ = rhs.file_content_;
    buf_idx_ = rhs.buf_idx_;
  }
  return *this;
}

Parser::~Parser() {}

void Parser::LoadFile(const std::string &filepath) {
  std::ifstream ifs(filepath.c_str());
  if (!ifs) {
    throw ParserException("Failed to open ifstream in LoadFile().");
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  file_content_ = ss.str();
}

void Parser::LoadData(const std::string &data) {
  file_content_ = data;
}

Config Parser::ParseConfig() {
  Config config;
  while (!IsEofReached()) {
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
    throw ParserException("Port has already set.");
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
  while (!IsEofReached() && GetC() != ';') {
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
  location_set_directives_.clear();
  LocationConf location;
  location.SetIsBackwardSearch(is_location_back);
  SkipSpaces();
  // Get path
  std::string path_pattern = GetWord();
  if (path_pattern.empty()) {
    throw ParserException("location's path pattern is empty.");
  }
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
      ParseErrorPageDirective(location);
    } else if (directive == "return") {
      ParseReturnDirective(location);
    } else {
      throw ParserException("Unknown directive in Location block.");
    }
    SkipSpaces();
    location_set_directives_.insert(directive);
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
  if (IsDirectiveSetInLocation("client_max_body_size")) {
    throw ParserException("client_max_body_size has already set.");
  }

  SkipSpaces();
  std::string body_size_str = GetWord();
  Result<unsigned long> result = utils::Stoul(body_size_str);
  if (result.IsErr()) {
    throw ParserException("Invalid body size.");
  }
  unsigned long body_size = result.Ok();
  location.SetClientMaxBodySize(body_size);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException(
        "Can't find semicolon after client_max_body_size directive.");
  }
}

void Parser::ParseRootDirective(LocationConf &location) {
  if (IsDirectiveSetInLocation("root")) {
    throw ParserException("root has already set.");
  }
  if (IsDirectiveSetInLocation("return")) {
    throw ParserException("root and return conflicts.");
  }

  SkipSpaces();
  std::string root = GetWord();
  location.SetRootDir(root);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after root directive.");
  }
}

void Parser::ParseIndexDirective(LocationConf &location) {
  if (IsDirectiveSetInLocation("index")) {
    throw ParserException("index has already set.");
  }
  if (IsDirectiveSetInLocation("return")) {
    throw ParserException("index and return conflicts.");
  }

  SkipSpaces();
  while (!IsEofReached() && GetC() != ';') {
    UngetC();
    std::string filepath = GetWord();
    location.AppendIndexPages(filepath);
    SkipSpaces();
  }
}

void Parser::ParseErrorPageDirective(LocationConf &location) {
  if (IsDirectiveSetInLocation("return")) {
    throw ParserException("error_page and return conflicts.");
  }
  SkipSpaces();
  // 最後のargがエラーページのfilepathになってる｡
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
    http::HttpStatus status =
        static_cast<http::HttpStatus>(atoi(args[i].c_str()));
    if (!http::StatusCodes::IsHttpStatus(status)) {
      throw ParserException("error_page directive arg %lu isn't valid number.",
                            status);
    }
    if (location.GetErrorPages().find(status) !=
        location.GetErrorPages().end()) {
      throw ParserException("error_page %d has already set.", status);
    }
    location.AppendErrorPages(status, error_page);
  }

  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after error_page directive.");
  }
}

void Parser::ParseAutoindexDirective(LocationConf &location) {
  if (IsDirectiveSetInLocation("autoindex")) {
    throw ParserException("autoindex has already set.");
  }
  if (IsDirectiveSetInLocation("return")) {
    throw ParserException("autoindex and return conflicts.");
  }

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
  if (IsDirectiveSetInLocation("is_cgi")) {
    throw ParserException("is_cgi has already set.");
  }
  if (location.GetAutoIndex()) {
    throw ParserException("is_cgi and 'autoindex on' conflicts.");
  }
  if (IsDirectiveSetInLocation("return")) {
    throw ParserException("is_cgi and return conflicts.");
  }

  SkipSpaces();
  std::string on_or_off = GetWord();
  bool is_cgi = ParseOnOff(on_or_off);
  location.SetIsCgi(is_cgi);
  SkipSpaces();
  if (GetC() != ';') {
    throw ParserException("Can't find semicolon after is_cgi directive.");
  }
}

void Parser::ParseReturnDirective(LocationConf &location) {
  if (IsDirectiveSetInLocation("return")) {
    throw ParserException("return has already set.");
  }
  if (IsDirectiveSetInLocation("is_cgi") ||
      IsDirectiveSetInLocation("autoindex") ||
      IsDirectiveSetInLocation("error_page") ||
      IsDirectiveSetInLocation("index") || IsDirectiveSetInLocation("root")) {
    throw ParserException(
        "return can't belong to location with other directives");
  }
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
  if (IsEofReached()) {
    return;
  }
  if (!isspace(GetC())) {
    UngetC();
    return;
  }
  while (!IsEofReached() && isspace(GetC())) {
  }
  if (!IsEofReached()) {
    UngetC();
  }
}

char Parser::GetC() {
  if (IsEofReached()) {
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
  while (!IsEofReached() &&
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
  if (method == http::method_strs::kGet || method == http::method_strs::kPost ||
      method == http::method_strs::kDelete) {
    return true;
  }
  return false;
}

// Response Status Code's format is defined in rfc7231.
// https://datatracker.ietf.org/doc/html/rfc7231#section-6
bool Parser::IsValidHttpStatusCode(const std::string &code) {
  if (utils::IsDigits(code) && code.length() == 3 &&
      (code[0] >= '1' && code[0] <= '5')) {
    return true;
  }
  return false;
}

bool Parser::IsValidPort(const std::string &port) {
  Result<unsigned long> result = utils::Stoul(port);
  return result.IsOk() && result.Ok() <= kMaxPortNumber;
}

bool Parser::ParseOnOff(const std::string &on_or_off) {
  if (on_or_off == "on") {
    return true;
  } else if (on_or_off == "off") {
    return false;
  }
  throw ParserException("ParseOnOff");
}

bool Parser::IsEofReached() {
  return buf_idx_ >= file_content_.length();
}

bool Parser::IsDirectiveSetInLocation(const std::string &directive) {
  return location_set_directives_.find(directive) !=
         location_set_directives_.end();
}

Parser::ParserException::ParserException(const char *errfmt, ...) {
  va_list args;
  va_start(args, errfmt);

  vsnprintf(errmsg_, MAX_ERROR_LEN, errfmt, args);
  va_end(args);
}

const char *Parser::ParserException::what() const throw() {
  return errmsg_;
}

}  // namespace config

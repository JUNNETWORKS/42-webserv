#include "cgi/cgi_request.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <set>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"

namespace cgi {

CgiRequest::CgiRequest(const std::string &request_path,
                       const std::string &query_string,
                       const http::HttpRequest &request,
                       const config::LocationConf &location)
    : cgi_pid_(-1),
      cgi_unisock_(-1),
      request_path_(request_path),
      query_string_(query_string),
      request_(request),
      location_(location) {}

CgiRequest::CgiRequest(const CgiRequest &rhs)
    : request_(rhs.request_), location_(rhs.location_) {
  *this = rhs;
}

CgiRequest &CgiRequest::operator=(const CgiRequest &rhs) {
  if (this != &rhs) {
    cgi_pid_ = rhs.cgi_pid_;
    cgi_unisock_ = rhs.cgi_unisock_;
    request_path_ = rhs.request_path_;
    cgi_path_ = rhs.cgi_path_;
    query_string_ = rhs.query_string_;
    cgi_args_ = rhs.cgi_args_;
    cgi_variables_ = rhs.cgi_variables_;
  }
  return *this;
}

CgiRequest::~CgiRequest() {}

// Getter, Setter
// ========================================================================
pid_t CgiRequest::GetPid() const {
  return cgi_pid_;
}

int CgiRequest::GetCgiUnisock() const {
  return cgi_unisock_;
}

const std::string &CgiRequest::GetCgiPath() const {
  return cgi_path_;
}

const std::string &CgiRequest::GetQueryString() const {
  return query_string_;
}

const std::vector<std::string> &CgiRequest::GetCgiArgs() const {
  return cgi_args_;
}

// RunCgi
// ========================================================================
bool CgiRequest::RunCgi() {
  bool result = false;
  result |= ParseCgiRequest();
  CreateCgiMetaVariablesFromHttpRequest(request_, location_);
  result |= ForkAndExecuteCgi();
  return result;
}

// Parse
// ========================================================================
bool CgiRequest::ParseCgiRequest() {
  std::string::size_type pos = request_path_.find("?");
  if (pos != std::string::npos) {
    cgi_path_ = request_path_.substr(0, pos);  // decode
    query_string_ = request_path_.substr(pos + 1);
    if (query_string_ != "" && query_string_.find('=') == std::string::npos) {
      // TODO : argv の 最大数でリミットかける必要ある？
      // TODO : cgi_args は パーセントデコーディングされる
      // この時 + は split されない
      cgi_args_ = utils::SplitString(query_string_, "+");
    }
  } else {
    cgi_path_ = request_path_;  // decode
    query_string_ = "";
  }
  return true;
}

// Exec Cgi
// ========================================================================
bool CgiRequest::ForkAndExecuteCgi() {
  int sockfds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfds) == -1) {
    return false;
  }
  // TODO: UnixDomainSocketを O_NONBLOCK に設定

  int parentsock = sockfds[0];
  int childsock = sockfds[1];

  cgi_pid_ = fork();
  if (cgi_pid_ < -1) {
    close(parentsock);
    close(childsock);
    return false;
  }
  if (cgi_pid_ == 0) {
    // Child
    close(parentsock);
    dup2(childsock, STDOUT_FILENO);
    close(childsock);
    ExecuteCgi();
    exit(EXIT_FAILURE);
  } else {
    // Parent
    cgi_unisock_ = parentsock;
    close(childsock);
    return true;
  }
}

void CgiRequest::ExecuteCgi() {
  UnsetAllEnvironmentVariables();
  SetMetaVariables();
  // TODO : stdin
  // TODO : chdir
  cgi_args_.insert(cgi_args_.begin(), cgi_path_);
  char **argv = alloc_dptr(cgi_args_);
  if (argv == NULL) {
    return;
  }
  execve(cgi_path_.c_str(), argv, environ);
  free_dptr(argv);
}

void CgiRequest::UnsetAllEnvironmentVariables() const {
  for (size_t i = 0; environ[i] != NULL; i++) {
    unsetenv(environ[i]);
  }
}

// 各変数の役割は以下のサイトを参照
// http://bashhp.web.fc2.com/WWW/header.html

// TODO: HTTP固有のメタ変数
// HTTP_ACCEPT
// HTTP_COOKIE
// HTTP_REFERER
// HTTP_USER_AGENT
// unsetenv("REMOTE_IDENT"); // IDENTプロトコル関係｡ サポートしない｡
// unsetenv("REMOTE_USER");  // IDENTプロトコル関係｡ サポートしない｡
void CgiRequest::CreateCgiMetaVariablesFromHttpRequest(
    const http::HttpRequest &request, const config::LocationConf &location) {
  (void)location;
  (void)request;
  // TODO
  cgi_variables_["SERVER_SOFTWARE"] = "webserv/1.0";
  cgi_variables_["SERVER_NAME"] = "";  // apache : 127.0.0.1
  cgi_variables_["GATEWAY_INTERFACE"] = "CGI/1.1";
  cgi_variables_["SERVER_PROTOCOL"] = "HTTP/1.1";  // TODO
  cgi_variables_["SERVER_PORT"] = "";              // TODO
  cgi_variables_["REQUEST_METHOD"] = request.GetMethod();
  cgi_variables_["HTTP_ACCEPT"] = "";  // TODO;
  cgi_variables_["PATH_INFO"] = "";
  cgi_variables_["PATH_TRANSLATED"] = "";  // unsetenv("PATH_TRANSLATED");
  cgi_variables_["SCRIPT_NAME"] = "";
  cgi_variables_["QUERY_STRING"] = query_string_;
  cgi_variables_["REMOTE_HOST"] = "";
  cgi_variables_["REMOTE_ADDR"] = "";
  cgi_variables_["REMOTE_USER"] = "";
  cgi_variables_["AUTH_TYPE"] = "";
  cgi_variables_["CONTENT_TYPE"] = "";
  cgi_variables_["CONTENT_LENGTH"] = "";
}

void CgiRequest::SetMetaVariables() {
  for (std::map<std::string, std::string>::const_iterator it =
           cgi_variables_.begin();
       it != cgi_variables_.end(); ++it) {
    setenv(it->first.c_str(), it->second.c_str(), 1);
  }
}

char **CgiRequest::alloc_dptr(const std::vector<std::string> &v) const {
  char **dptr;

  dptr = (char **)malloc(sizeof(char *) * (v.size() + 1));
  if (dptr == NULL) {
    return NULL;
  }
  size_t i = 0;
  for (; i < v.size(); i++) {
    dptr[i] = strdup(v[i].c_str());
    if (dptr[i] == NULL) {
      free_dptr(dptr);
      return (NULL);
    }
  }
  dptr[i] = NULL;
  return (dptr);
}

void CgiRequest::free_dptr(char **dptr) const {
  for (size_t i = 0; dptr[i]; i++) {
    free(dptr[i]);
  }
  free(dptr);
}

}  // namespace cgi

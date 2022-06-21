#include "cgi/cgi_request.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <set>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"

namespace cgi {

CgiRequest::CgiRequest(const std::string &cgi_path,
                       const http::HttpRequest &request,
                       const config::LocationConf &location)
    : cgi_path_(cgi_path) {
  CreateCgiMetaVariablesFromHttpRequest(request, location);
}

CgiRequest::CgiRequest(const CgiRequest &rhs) {
  *this = rhs;
}

CgiRequest &CgiRequest::operator=(const CgiRequest &rhs) {
  if (this != &rhs) {
    cgi_path_ = rhs.cgi_path_;
    cgi_variables_ = rhs.cgi_variables_;
  }
  return *this;
}

CgiRequest::~CgiRequest() {}

int CgiRequest::ForkAndExecuteCgi() {
  int sockfds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfds) == -1) {
    // Error
    exit(EXIT_FAILURE);  // TODO :
  }

  int parentsock = sockfds[0];
  int childsock = sockfds[1];

  pid_t pid = fork();
  if (pid == 0) {
    // Child
    close(parentsock);
    dup2(childsock, STDOUT_FILENO);
    close(childsock);
    ExecuteCgi();
    exit(EXIT_FAILURE);  // TODO :
  } else if (pid > 0) {
    // Parent
    close(childsock);
    return parentsock;
  } else {
    // Error
    exit(EXIT_FAILURE);  // TODO :
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
  cgi_variables_["REQUEST_METHOD"] = request.();
  cgi_variables_["HTTP_ACCEPT"] = "";  // TODO;
  cgi_variables_["PATH_INFO"] = "";
  cgi_variables_["PATH_TRANSLATED"] = "";  // unsetenv("PATH_TRANSLATED");
  cgi_variables_["SCRIPT_NAME"] = "";
  cgi_variables_["QUERY_STRING"] = "";
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

void CgiRequest::ExecuteCgi() {
  // TODO : 不要な環境変数のunset
  SetMetaVariables();
  // TODO : stdin
  // TODO : chdir 必要？
  char **argv = new char *[2];
  argv[0] = strdup(cgi_path_.c_str());  // TODO : req の pathにする必要あいr
  argv[1] = NULL;
  execve(cgi_path_.c_str(), argv, environ);
}

}  // namespace cgi

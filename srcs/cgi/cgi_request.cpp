#include "cgi/cgi_request.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <set>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"

namespace cgi {

CgiRequest::CgiRequest(const std::string &cgi_path, http::HttpRequest &request,
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
    exit(EXIT_FAILURE);
  }

  int parentsock = sockfds[0];
  int childsock = sockfds[1];

  pid_t pid = fork();
  if (pid == 0) {
    // Child
    close(parentsock);
    ExecuteCgi();
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    // Parent
    close(childsock);
    return parentsock;
  } else {
    // Error
    exit(EXIT_FAILURE);
  }
}

// 各変数の役割は以下のサイトを参照
// http://bashhp.web.fc2.com/WWW/header.html
void CgiRequest::CreateCgiMetaVariablesFromHttpRequest(
    http::HttpRequest &request, const config::LocationConf &location) {
  (void)location;
  // unsetenv してるメタ変数は必須じゃない

  unsetenv("AUTH_TYPE");

  std::string length = request.GetHeader("CONTENT_LENGTH")[0];
  cgi_variables_["CONTENT_LENGTH"] = length.c_str();

  std::string content_type = request.GetHeader("CONTENT_TYPE")[0];
  cgi_variables_["CONTENT_TYPE"] = content_type.c_str();

  cgi_variables_["GATEWAY_INTERFACE"] = "CGI/1.1";

  // TODO: path から location.path 情報を削除した相対パスをセットする
  std::string path = request.GetPath();
  cgi_variables_["PATH_INFO"] = "";

  unsetenv("PATH_TRANSLATED");

  cgi_variables_["QUERY_STRING"] = "";

  cgi_variables_["REMOTE_ADDR"] = "";

  cgi_variables_["REMOTE_HOST"] = "";

  // IDENTプロトコル関係｡ webserv ではサポートしない｡
  unsetenv("REMOTE_IDENT");

  // IDENTプロトコル関係｡ webserv ではサポートしない｡
  unsetenv("REMOTE_USER");

  cgi_variables_["REQUEST_METHOD"] = "";

  cgi_variables_["SCRIPT_NAME"] = "";

  cgi_variables_["SERVER_NAME"] = "";

  cgi_variables_["SERVER_PORT"] = "";

  cgi_variables_["SERVER_PROTOCOL"] = "";

  cgi_variables_["SERVER_SOFTWARE"] = "webserv/1.0";

  // TODO: HTTP固有のメタ変数
  // HTTP_ACCEPT
  // HTTP_COOKIE
  // HTTP_REFERER
  // HTTP_USER_AGENT
}

void CgiRequest::SetMetaVariables() {
  for (std::map<std::string, std::string>::const_iterator it =
           cgi_variables_.begin();
       it != cgi_variables_.end(); ++it) {
    setenv(it->first.c_str(), it->second.c_str(), 1);
  }
}

void CgiRequest::ExecuteCgi() {
  SetMetaVariables();
  char **argv = new char *[2];
  argv[0] = strdup(cgi_path_.c_str());
  argv[1] = NULL;
  execve(cgi_path_.c_str(), argv, environ);
}

}  // namespace cgi

#include "cgi/cgi_request.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <set>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"

namespace cgi {

CgiMetaVariables CreateCgiMetaVariables(const server::ConnSocket *conn_sock,
                                        const http::HttpRequest &request) {
  CgiMetaVariables variables;

  // SERVER_NAME
  Result<const std::vector<std::string> &> host_res = request.GetHeader("Host");
  if (host_res.IsOk() && !host_res.Ok().empty()) {
    variables.server_name = host_res.Ok()[0];
  } else {
    variables.server_name = conn_sock->GetServerIp();
  }

  // SERVER_PORT
  variables.server_port = conn_sock->GetServerPort();

  // REMOTE_HOST
  if (!conn_sock->GetRemoteName().empty()) {
    variables.remote_host = conn_sock->GetRemoteName();
  } else {
    variables.remote_host = conn_sock->GetRemoteIp();
  }

  // REMOTE_ADDR
  variables.remote_addr = conn_sock->GetRemoteIp();

  // CONTENT_TYPE
  Result<const std::vector<std::string> &> content_type_res =
      request.GetHeader("Content-Type");
  if (content_type_res.IsOk() && !content_type_res.Ok().empty()) {
    variables.content_type = content_type_res.Ok()[0];
  }

  // CONTENT_LENGTH
  Result<const std::vector<std::string> &> content_length_res =
      request.GetHeader("Content-Length");
  if (content_length_res.IsOk() && !content_length_res.Ok().empty()) {
    variables.content_length = content_length_res.Ok()[0];
  }

  return variables;
}

CgiRequest::CgiRequest(const server::ConnSocket *conn_sock,
                       const http::HttpRequest &request,
                       const config::LocationConf &location)
    : cgi_pid_(-1),
      cgi_unisock_(-1),
      request_path_(request.GetPath()),
      query_string_(request.GetQueryParam()),
      cgi_meta_variables_(CreateCgiMetaVariables(conn_sock, request)),
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
    query_string_ = rhs.query_string_;
    script_name_ = rhs.script_name_;
    exec_cgi_script_path_ = rhs.exec_cgi_script_path_;
    path_info_ = rhs.path_info_;
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
  return request_path_;
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
  if (!ParseQueryString()) {
    return false;
  }
  if (!SplitIntoCgiPathAndPathInfo()) {
    return false;
  }
  return true;
}

bool CgiRequest::ParseQueryString() {
  if (query_string_ == "") {
    return true;
  }
  if (query_string_.find('=') != std::string::npos) {
    return true;
  }
  cgi_args_ = utils::SplitString(query_string_, "+");
  for (std::vector<std::string>::iterator it = cgi_args_.begin();
       it != cgi_args_.end(); it++) {
    Result<std::string> res = utils::PercentDecode(*it);
    if (res.IsErr()) {
      return false;
    }
    *it = res.Ok();
  }
  return true;
}

bool CgiRequest::SplitIntoCgiPathAndPathInfo() {
  std::vector<std::string> file_vec =
      utils::SplitString(location_.GetAfterLocation(request_path_), "/");

  std::string exec_cgi_path = location_.GetRootDir();
  std::string script_name = "";
  for (std::vector<std::string>::const_iterator it = file_vec.begin();
       it != file_vec.end(); it++) {
    if (*it == "") {
      continue;
    }
    script_name = utils::JoinPath(script_name, *it);
    exec_cgi_path = utils::JoinPath(exec_cgi_path, script_name);
    utils::File f(exec_cgi_path);
    // TODO : 実行権限も確認
    if (f.GetFileType() == utils::File::kFile) {
      script_name_ = utils::JoinPath(location_.GetPathPattern(), script_name);
      exec_cgi_script_path_ = exec_cgi_path;
      path_info_ = request_path_;
      path_info_ = path_info_.replace(0, script_name_.length(), "");
      return true;
    }
  }
  return false;
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
  if (!MoveToExecuteCgiDir(exec_cgi_script_path_)) {
    return;
  }
  cgi_args_.insert(cgi_args_.begin(), script_name_);
  char **argv = alloc_dptr(cgi_args_);
  if (argv == NULL) {
    return;
  }
  execve(exec_cgi_script_path_.c_str(), argv, environ);
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
  cgi_variables_["SERVER_SOFTWARE"] = "webserv/1.0";
  cgi_variables_["SERVER_NAME"] = cgi_meta_variables_.server_name;
  cgi_variables_["GATEWAY_INTERFACE"] = "CGI/1.1";
  cgi_variables_["SERVER_PROTOCOL"] = "HTTP/1.1";
  cgi_variables_["SERVER_PORT"] = cgi_meta_variables_.server_port;
  cgi_variables_["REQUEST_METHOD"] = request.GetMethod();
  cgi_variables_["HTTP_ACCEPT"] = "*/*";
  cgi_variables_["PATH_INFO"] = path_info_;
  cgi_variables_["PATH_TRANSLATED"] = "";  // unsetenv("PATH_TRANSLATED");
  cgi_variables_["SCRIPT_NAME"] = script_name_;
  cgi_variables_["QUERY_STRING"] = query_string_;
  cgi_variables_["REMOTE_HOST"] = cgi_meta_variables_.remote_host;
  cgi_variables_["REMOTE_ADDR"] = cgi_meta_variables_.remote_addr;
  cgi_variables_["AUTH_TYPE"] = "";
  cgi_variables_["CONTENT_TYPE"] = cgi_meta_variables_.content_type;
  cgi_variables_["CONTENT_LENGTH"] = cgi_meta_variables_.content_length;

  // HTTPヘッダーは "HTTP_" prefix を付けて環境変数にセット
  const http::HeaderMap &http_headers = request.GetHeaders();
  for (http::HeaderMap::const_iterator it = http_headers.begin();
       it != http_headers.end(); ++it) {
    std::string header_key = "HTTP_" + it->first;
    std::string header_value;
    for (std::vector<std::string>::const_iterator vit = it->second.begin();
         vit != it->second.end(); ++vit) {
      header_value += *vit + ",";
    }
    if (!header_value.empty()) {
      header_value.erase(header_value.size() - 1);  // 最後の ',' を削除
      cgi_variables_[header_key] = header_value;
    }
  }
}

void CgiRequest::SetMetaVariables() {
  for (std::map<std::string, std::string>::const_iterator it =
           cgi_variables_.begin();
       it != cgi_variables_.end(); ++it) {
    setenv(it->first.c_str(), it->second.c_str(), 1);
  }
}

bool CgiRequest::MoveToExecuteCgiDir(
    const std::string &exec_cgi_script_path_) const {
  Result<std::string> result =
      utils::NormalizePath(utils::JoinPath(exec_cgi_script_path_, ".."));
  if (result.IsErr()) {
    return false;
  }
  std::string exec_dir = result.Ok();
  if (chdir(exec_dir.c_str())) {
    return false;
  }
  return true;
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

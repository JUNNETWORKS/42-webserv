#include "cgi/cgi_request.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <set>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"
#include "utils/io.hpp"

namespace cgi {

CgiRequest::CgiRequest() : cgi_pid_(-1), cgi_unisock_(-1) {}

CgiRequest::CgiRequest(const CgiRequest &rhs) {
  *this = rhs;
}

CgiRequest &CgiRequest::operator=(const CgiRequest &rhs) {
  if (this != &rhs) {
    cgi_pid_ = rhs.cgi_pid_;
    cgi_unisock_ = rhs.cgi_unisock_;
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

const std::vector<std::string> &CgiRequest::GetCgiArgs() const {
  return cgi_args_;
}

// RunCgi
// ========================================================================
bool CgiRequest::RunCgi(const server::ConnSocket *conn_sock,
                        const http::HttpRequest &request,
                        const config::LocationConf &location) {
  bool result = true;
  result &= ParseQueryString(request);
  result &= DetermineExecutionCgiPath(request, location);
  CreateCgiMetaVariablesFromHttpRequest(conn_sock, request, location);
  result &= ForkAndExecuteCgi();
  return result;
}

// Parse
// ========================================================================
bool CgiRequest::ParseQueryString(const http::HttpRequest &request) {
  std::string query_string = request.GetQueryParam();
  if (query_string == "") {
    return true;
  }
  if (query_string.find('=') != std::string::npos) {
    return true;
  }
  cgi_args_ = utils::SplitString(query_string, "+");
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

bool CgiRequest::DetermineExecutionCgiPath(
    const http::HttpRequest &request, const config::LocationConf &location) {
  typedef std::vector<std::pair<std::string, std::string> > StringPairVec;

  StringPairVec possible_scipts;

  std::string request_path = request.GetPath();
  std::vector<std::string> file_vec =
      utils::SplitString(location.RemovePathPatternFromPath(request_path), "/");

  {
    std::string script_name = "";
    std::string exec_cgi_path = location.GetRootDir();
    for (std::vector<std::string>::const_iterator it = file_vec.begin();
         it != file_vec.end(); it++) {
      if (*it == "") {
        continue;
      }
      script_name = utils::JoinPath(script_name, *it);
      exec_cgi_path = utils::JoinPath(exec_cgi_path, script_name);
      possible_scipts.push_back(std::make_pair(script_name, exec_cgi_path));
    }
  }

  {
    // index が設定されている場合は実行できるか確認する
    std::string script_name = "";
    std::string exec_cgi_path = location.GetRootDir();
    const std::vector<std::string> &index_pages = location.GetIndexPages();
    for (std::vector<std::string>::const_iterator it = index_pages.begin();
         it != index_pages.end(); ++it) {
      script_name = utils::JoinPath(location.GetPathPattern(), *it);
      exec_cgi_path = utils::JoinPath(location.GetRootDir(), *it);
      possible_scipts.push_back(std::make_pair(script_name, exec_cgi_path));
    }
  }

  for (StringPairVec::const_iterator it = possible_scipts.begin();
       it != possible_scipts.end(); ++it) {
    std::string script_name = it->first;
    std::string exec_cgi_path = it->second;
    if (utils::IsExecutableFile(exec_cgi_path)) {
      script_name_ = utils::JoinPath(location.GetPathPattern(), script_name);
      exec_cgi_script_path_ = exec_cgi_path;
      path_info_ = request_path;
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
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockfds) == -1) {
    return false;
  }

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
    if (dup2(childsock, STDIN_FILENO) < 0 ||
        dup2(childsock, STDOUT_FILENO) < 0) {
      close(parentsock);
      close(childsock);
      exit(EXIT_FAILURE);
    }
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
  if (!MoveToCgiExecutionDir(exec_cgi_script_path_)) {
    return;
  }
  cgi_args_.insert(cgi_args_.begin(), script_name_);
  char **argv = AllocCharPtrsFromVectorString(cgi_args_);
  if (argv == NULL) {
    return;
  }
  execve(exec_cgi_script_path_.c_str(), argv, environ);
  FreePtrArray(argv);
}

void CgiRequest::UnsetAllEnvironmentVariables() const {
  for (size_t i = 0; environ[i] != NULL; i++) {
    unsetenv(environ[i]);
  }
}

// 各変数の役割は以下のサイトを参照
// http://bashhp.web.fc2.com/WWW/header.html
void CgiRequest::CreateCgiMetaVariablesFromHttpRequest(
    const server::ConnSocket *conn_sock, const http::HttpRequest &request,
    const config::LocationConf &location) {
  cgi_variables_["SERVER_SOFTWARE"] = "webserv/1.0";
  cgi_variables_["GATEWAY_INTERFACE"] = "CGI/1.1";
  cgi_variables_["SERVER_PROTOCOL"] = request.GetHttpVersion();
  cgi_variables_["REQUEST_METHOD"] = request.GetMethod();
  cgi_variables_["PATH_INFO"] = path_info_;
  if (!path_info_.empty()) {
    // ここはApacheと結果が異なる｡
    cgi_variables_["PATH_TRANSLATED"] =
        location.GetAbsolutePath(location.GetPathPattern() + path_info_);
  }
  cgi_variables_["SCRIPT_NAME"] = script_name_;
  cgi_variables_["QUERY_STRING"] = request.GetQueryParam();
  cgi_variables_["AUTH_TYPE"] = "";

  CreateCgiNetworkVariables(conn_sock, request);
  CreateCgiContentVariables(request);
  // HTTPヘッダーのメタ変数を作成
  CreateCgiHttpVariables(request);
}

void CgiRequest::CreateCgiContentVariables(const http::HttpRequest &request) {
  // CONTENT_TYPE
  Result<const std::vector<std::string> &> content_type_res =
      request.GetHeader("Content-Type");
  if (content_type_res.IsOk() && !content_type_res.Ok().empty()) {
    cgi_variables_["CONTENT_TYPE"] = content_type_res.Ok()[0];
  }

  // CONTENT_LENGTH
  Result<const std::vector<std::string> &> content_length_res =
      request.GetHeader("Content-Length");
  if (content_length_res.IsOk() && !content_length_res.Ok().empty()) {
    cgi_variables_["CONTENT_LENGTH"] = content_length_res.Ok()[0];
  }
}

void CgiRequest::CreateCgiHttpVariables(const http::HttpRequest &request) {
  // HTTPヘッダーは "HTTP_" prefix を付けて環境変数にセット
  const http::HeaderMap &http_headers = request.GetHeaders();
  for (http::HeaderMap::const_iterator it = http_headers.begin();
       it != http_headers.end(); ++it) {
    std::string tmp = it->first;
    std::replace(tmp.begin(), tmp.end(), '-', '_');
    std::string header_key = "HTTP_" + tmp;
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

void CgiRequest::CreateCgiNetworkVariables(const server::ConnSocket *conn_sock,
                                           const http::HttpRequest &request) {
  // SERVER_NAME
  Result<const std::vector<std::string> &> host_res = request.GetHeader("Host");
  if (host_res.IsOk() && !host_res.Ok().empty()) {
    cgi_variables_["SERVER_NAME"] = host_res.Ok()[0];
  } else {
    cgi_variables_["SERVER_NAME"] = conn_sock->GetServerIp();
  }

  // SERVER_PORT
  cgi_variables_["SERVER_PORT"] = conn_sock->GetServerPort();

  // REMOTE_HOST
  if (!conn_sock->GetRemoteName().empty()) {
    cgi_variables_["REMOTE_HOST"] = conn_sock->GetRemoteName();
  } else {
    cgi_variables_["REMOTE_HOST"] = conn_sock->GetRemoteIp();
  }

  // REMOTE_ADDR
  cgi_variables_["REMOTE_ADDR"] = conn_sock->GetRemoteIp();
}

void CgiRequest::SetMetaVariables() {
  for (std::map<std::string, std::string>::const_iterator it =
           cgi_variables_.begin();
       it != cgi_variables_.end(); ++it) {
    setenv(it->first.c_str(), it->second.c_str(), 1);
  }
}

bool CgiRequest::MoveToCgiExecutionDir(
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

char **CgiRequest::AllocCharPtrsFromVectorString(
    const std::vector<std::string> &v) const {
  char **dptr;

  dptr = (char **)malloc(sizeof(char *) * (v.size() + 1));
  if (dptr == NULL) {
    return NULL;
  }
  size_t i = 0;
  for (; i < v.size(); i++) {
    dptr[i] = strdup(v[i].c_str());
    if (dptr[i] == NULL) {
      FreePtrArray(dptr);
      return (NULL);
    }
  }
  dptr[i] = NULL;
  return (dptr);
}

void CgiRequest::FreePtrArray(char **dptr) const {
  for (size_t i = 0; dptr[i]; i++) {
    free(dptr[i]);
  }
  free(dptr);
}

}  // namespace cgi

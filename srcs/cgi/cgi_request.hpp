#ifndef CGI_CGI_REQUEST_HPP_
#define CGI_CGI_REQUEST_HPP_

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <map>
#include <set>
#include <vector>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"
#include "result/result.hpp"
#include "server/socket.hpp"
#include "utils/File.hpp"
#include "utils/path.hpp"

namespace cgi {

using namespace result;

class CgiRequest {
 private:
  pid_t cgi_pid_;
  int cgi_unisock_;
  std::string script_name_;
  std::string exec_cgi_script_path_;
  std::string path_info_;
  std::vector<std::string> cgi_args_;
  std::map<std::string, std::string> cgi_variables_;

 public:
  CgiRequest();
  CgiRequest(const CgiRequest &rhs);
  CgiRequest &operator=(const CgiRequest &rhs);
  ~CgiRequest();

  pid_t GetPid() const;
  int GetCgiUnisock() const;
  const std::vector<std::string> &GetCgiArgs() const;

  bool RunCgi(const server::ConnSocket *conn_sock,
              const http::HttpRequest &request,
              const config::LocationConf &location);

 private:
  bool ParseQueryString(const http::HttpRequest &request);
  bool DetermineExecutionCgiPath(const http::HttpRequest &request,
                                 const config::LocationConf &location);
  Result<std::string> SearchCgiPath(const std::string &base_path,
                                    const std::vector<std::string> &v,
                                    const config::LocationConf &location);

  // 返り値は無名ドメインソケットのfd
  bool ForkAndExecuteCgi();

  // リクエストからCGIスクリプトに渡す変数を作成する
  void CreateCgiMetaVariablesFromHttpRequest(
      const server::ConnSocket *conn_sock, const http::HttpRequest &request,
      const config::LocationConf &location);
  void CreateCgiContentVariables(const http::HttpRequest &request);
  void CreateCgiNetworkVariables(const server::ConnSocket *conn_sock,
                                 const http::HttpRequest &request);
  void CreateCgiHttpVariables(const http::HttpRequest &request);
  void UnsetAllEnvironmentVariables() const;

  void SetMetaVariables();

  void ExecuteCgi();
  bool MoveToCgiExecutionDir(const std::string &exec_cgi_script_path_) const;

  // TODO : どこかに移動させる。
  // exec なども移動させたい。
  char **AllocCharPtrsFromVectorString(const std::vector<std::string> &v) const;
  void FreePtrArray(char **dptr) const;
};

}  // namespace cgi
#endif

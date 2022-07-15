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

struct CgiMetaVariables {
  std::string server_name;  // ホストのIP
  std::string server_port;  // listenポート
  std::string remote_host;  // クライアントのホスト名
  std::string remote_addr;  // クライアントのIPアドレス
  std::string content_type;
  std::string content_length;  // chunked request の時は空に設定する
};

CgiMetaVariables CreateCgiMetaVariables(const server::ConnSocket *conn_sock,
                                        const http::HttpRequest &request);

class CgiRequest {
 private:
  pid_t cgi_pid_;
  int cgi_unisock_;
  std::string request_path_;
  std::string query_string_;
  std::string script_name_;
  std::string exec_cgi_script_path_;
  std::string path_info_;
  CgiMetaVariables cgi_meta_variables_;
  // TODO : 必要な情報だけ取るようにして、メンバ変数からは削除予定
  const http::HttpRequest &request_;
  // TODO : 必要な情報だけ取るようにして、メンバ変数からは削除予定
  const config::LocationConf &location_;
  std::vector<std::string> cgi_args_;
  std::map<std::string, std::string> cgi_variables_;

 public:
  CgiRequest(const server::ConnSocket *conn_sock,
             const http::HttpRequest &request,
             const config::LocationConf &location);
  CgiRequest(const CgiRequest &rhs);
  CgiRequest &operator=(const CgiRequest &rhs);
  ~CgiRequest();

  pid_t GetPid() const;
  int GetCgiUnisock() const;
  const std::string &GetCgiPath() const;
  const std::string &GetQueryString() const;
  const std::vector<std::string> &GetCgiArgs() const;

  bool RunCgi();

  // 現在はテストしたので、public
  bool ParseCgiRequest();

 private:
  CgiRequest();

  bool ParseQueryString();
  bool SplitIntoCgiPathAndPathInfo();

  // 返り値は無名ドメインソケットのfd
  bool ForkAndExecuteCgi();

  // リクエストからCGIスクリプトに渡す変数を作成する
  void CreateCgiMetaVariablesFromHttpRequest(
      const http::HttpRequest &request, const config::LocationConf &location);
  void UnsetAllEnvironmentVariables() const;

  void SetMetaVariables();

  void ExecuteCgi();
  bool MoveToExecuteCgiDir(const std::string &exec_cgi_script_path_) const;

  // TODO : どこかに移動させる。
  // exec なども移動させたい。
  char **alloc_dptr(const std::vector<std::string> &v) const;
  void free_dptr(char **dptr) const;
};

}  // namespace cgi
#endif

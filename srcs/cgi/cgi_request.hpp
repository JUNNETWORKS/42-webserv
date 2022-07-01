#ifndef CGI_CGI_REQUEST_HPP_
#define CGI_CGI_REQUEST_HPP_

#include <map>
#include <set>
#include <vector>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"

namespace cgi {

class CgiRequest {
 private:
  pid_t cgi_pid_;
  int cgi_unisock_;
  std::string request_path_;
  // TODO : 必要な情報だけ取るようにして、メンバ変数からは削除予定
  const http::HttpRequest &request_;
  // TODO : 必要な情報だけ取るようにして、メンバ変数からは削除予定
  const config::LocationConf &location_;
  std::string cgi_path_;
  std::string query_string_;
  std::vector<std::string> cgi_args_;
  std::map<std::string, std::string> cgi_variables_;

 public:
  CgiRequest(const std::string &request_path, const http::HttpRequest &request,
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
  bool ParseCgiRequest();
  // 返り値は無名ドメインソケットのfd
  // TODO: 第二引数はhttprequestじゃなくてcgirequestとかのほうがよいかも
  // TODO: cgiのpidをどうやってwaitするかが問題｡
  bool ForkAndExecuteCgi();

 private:
  CgiRequest();

  // リクエストからCGIスクリプトに渡す変数を作成する
  void CreateCgiMetaVariablesFromHttpRequest(
      const http::HttpRequest &request, const config::LocationConf &location);

  void SetMetaVariables();

  void ExecuteCgi();
};

}  // namespace cgi
#endif

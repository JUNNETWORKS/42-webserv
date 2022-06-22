#ifndef CGI_CGI_REQUEST_HPP_
#define CGI_CGI_REQUEST_HPP_

#include <map>
#include <set>

#include "config/location_conf.hpp"
#include "http/http_request.hpp"

namespace cgi {

class CgiRequest {
 private:
  std::string cgi_path_;
  std::string query_string_;
  std::map<std::string, std::string> cgi_variables_;

 public:
  CgiRequest(const std::string &request_path, const http::HttpRequest &request,
             const config::LocationConf &location);
  CgiRequest(const CgiRequest &rhs);
  CgiRequest &operator=(const CgiRequest &rhs);
  ~CgiRequest();

  const std::string &GetCgiPath() const;
  const std::string &GetQueryString() const;

  // 返り値は無名ドメインソケットのfd
  // TODO: 第二引数はhttprequestじゃなくてcgirequestとかのほうがよいかも
  // TODO: cgiのpidをどうやってwaitするかが問題｡
  int ForkAndExecuteCgi();

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

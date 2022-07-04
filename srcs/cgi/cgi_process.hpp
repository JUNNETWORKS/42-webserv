#ifndef CGI_CGI_PROCESS_HPP_
#define CGI_CGI_PROCESS_HPP_

#include "cgi/cgi_request.hpp"
#include "cgi/cgi_response.hpp"

namespace cgi {

struct CgiExecution {};

using namespace server;

class CgiProcess {
 private:
  static const unsigned long kDataPerRead = 1024;   // 1KB
  static const unsigned long kDataPerWrite = 1024;  // 1KB

  CgiRequest *cgi_request_;
  CgiResponse *cgi_response_;

  utils::ByteVector cgi_input_buffer_;
  utils::ByteVector cgi_output_buffer_;

  const config::LocationConf *location_;
  Epoll *epoll_;

  bool is_executed_;
  bool is_err_;
  bool is_finished_;

  // HttpCgiResponse と Epoll から参照されるので､
  // use-after-free を防ぐために参照カウントのようなものを持たせる｡
  bool is_unregistered_;

 public:
  CgiProcess(const config::LocationConf *location, Epoll *epoll);
  ~CgiProcess();

  // Cgiプロセスを作成し､UnixDomainSocketをEpollに登録する｡
  Result<void> RunCgi(http::HttpRequest &request);

  // Cgiプロセスをkillする
  void KillCgi();

  //========================================================================
  // Getter and Setter
  bool IsCgiExecuted() const;
  bool IsCgiFinished() const;
  bool IsUnregistered() const;
  void SetIsUnregistered(bool is_unregistered);
  CgiResponse *GetCgiResponse();

 private:
  CgiProcess(const CgiProcess &rhs);
  const CgiProcess &operator=(const CgiProcess &rhs);

  CgiRequest *AllocateCgiRequest(http::HttpRequest &request);

  static void HandleCgiEvent(FdEvent *fde, unsigned int events, void *data,
                             Epoll *epoll);
};

}  // namespace cgi

#endif

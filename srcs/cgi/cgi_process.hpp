#ifndef CGI_CGI_PROCESS_HPP_
#define CGI_CGI_PROCESS_HPP_

#include "cgi/cgi_request.hpp"
#include "cgi/cgi_response.hpp"

namespace cgi {

using namespace server;

class CgiProcess {
 public:
  enum StatusTypes {
    kExecuted = 1,
    kFinished = 1 << 1,
    // HttpCgiResponse と Epoll から参照されるので､
    // use-after-free を防ぐために参照カウントのようなものを持たせる｡
    kRemovable = 1 << 2
  };

 private:
  static const unsigned long kDataPerRead = 1024;   // 1KB
  static const unsigned long kDataPerWrite = 1024;  // 1KB
  static const long kUnisockTimeout = 5 * 1000;     // 5[sec]

  CgiRequest *cgi_request_;
  CgiResponse *cgi_response_;

  utils::ByteVector cgi_input_buffer_;
  utils::ByteVector cgi_output_buffer_;

  const config::LocationConf *location_;
  Epoll *epoll_;

  FdEvent *fde_;

  int status_;
  ConnSocket *socket_;

 public:
  CgiProcess(const config::LocationConf *location, Epoll *epoll,
             ConnSocket *socket);
  ~CgiProcess();

  // Cgiプロセスを作成し､UnixDomainSocketをEpollに登録する｡
  http::HttpStatus RunCgi(server::ConnSocket *conn_sock,
                          http::HttpRequest &request);

  // Cgiプロセスをkillする
  void KillCgi();

  //========================================================================
  // Getter and Setter
  bool IsCgiExecuted() const;
  bool IsRemovable() const;

  void SetIsExecuted(bool is_executed);
  void SetIsRemovable(bool is_unregistered);

  CgiResponse *GetCgiResponse() const;
  FdEvent *GetFde() const;

 private:
  CgiProcess(const CgiProcess &rhs);
  const CgiProcess &operator=(const CgiProcess &rhs);

  CgiRequest *AllocateCgiRequest(http::HttpRequest &request);

  static void HandleCgiEvent(FdEvent *fde, unsigned int events, void *data,
                             Epoll *epoll);
  static bool HandleCgiWriteEvent(CgiProcess *cgi_process, FdEvent *fde,
                                  Epoll *epoll);
  static bool HandleCgiReadEvent(CgiProcess *cgi_process);
};

}  // namespace cgi

#endif

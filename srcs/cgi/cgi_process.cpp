#include "cgi/cgi_process.hpp"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace cgi {

CgiProcess::CgiProcess(const config::LocationConf *location, Epoll *epoll)
    : cgi_request_(NULL),
      cgi_response_(NULL),
      cgi_input_buffer_(),
      cgi_output_buffer_(),
      location_(location),
      epoll_(epoll),
      fde_(NULL),
      status_(0) {}

CgiProcess::~CgiProcess() {
  if (cgi_request_) {
    // Cgiプロセスが生存している場合はKILL
    int wstats;
    if (waitpid(cgi_request_->GetPid(), &wstats, WNOHANG) == 0) {
      // KILL & wait
      KillCgi();
      waitpid(cgi_request_->GetPid(), &wstats, 0);
    }
  }

  if (fde_) {
    // Unixドメインソケットを閉じる
    close(fde_->fd);
  }

  delete fde_;
  delete cgi_request_;
  delete cgi_response_;
}

Result<void> CgiProcess::RunCgi(server::ConnSocket *conn_sock,
                                http::HttpRequest &request) {
  SetIsExecuted(true);

  cgi_request_ = new cgi::CgiRequest();
  cgi_response_ = new CgiResponse();
  // TODO: fork した後 execve に失敗した時のエラー検知と処理
  if (!cgi_request_->RunCgi(conn_sock, request, *location_) ||
      cgi_request_->GetCgiUnisock() < 0) {
    delete cgi_request_;
    delete cgi_response_;
    cgi_request_ = NULL;
    cgi_response_ = NULL;
    return Error();
  }
  cgi_input_buffer_.insert(cgi_input_buffer_.begin(), request.GetBody().begin(),
                           request.GetBody().end());

  FdEvent *fde =
      CreateFdEvent(cgi_request_->GetCgiUnisock(), HandleCgiEvent, this);
  epoll_->Register(fde);
  if (!cgi_input_buffer_.empty()) {
    epoll_->Add(fde, kFdeWrite);
  }
  epoll_->Add(fde, kFdeRead);
  epoll_->SetTimeout(fde, kUnisockTimeout);
  fde_ = fde;
  return Result<void>();
}

void CgiProcess::KillCgi() {
  kill(cgi_request_->GetPid(), SIGKILL);
}

bool CgiProcess::IsCgiExecuted() const {
  return status_ & kExecuted;
}

bool CgiProcess::IsRemovable() const {
  return status_ & kRemovable;
}

void CgiProcess::SetIsExecuted(bool is_executed) {
  if (is_executed) {
    status_ |= kExecuted;
  } else {
    status_ &= ~kExecuted;
  }
}

void CgiProcess::SetIsRemovable(bool is_unregistered) {
  if (is_unregistered) {
    status_ |= kRemovable;
  } else {
    status_ &= ~kRemovable;
  }
}

CgiResponse *CgiProcess::GetCgiResponse() const {
  return cgi_response_;
}

FdEvent *CgiProcess::GetFde() const {
  return fde_;
}

namespace {

void DeleteCgiProcess(Epoll *epoll, FdEvent *fde) {
  CgiProcess *cgi_process = reinterpret_cast<CgiProcess *>(fde->data);

  if (cgi_process->IsRemovable()) {
    epoll->Unregister(fde);
    delete cgi_process;
  } else {
    epoll->Unregister(fde);
    cgi_process->SetIsRemovable(true);
  }
  return;
}

}  // namespace

void CgiProcess::HandleCgiEvent(FdEvent *fde, unsigned int events, void *data,
                                Epoll *epoll) {
  printf("HandleCgiEvent()\n");

  CgiProcess *cgi_process = reinterpret_cast<CgiProcess *>(data);

  CgiRequest *cgi_request = cgi_process->cgi_request_;
  CgiResponse *cgi_response = cgi_process->cgi_response_;

  if (cgi_process->IsRemovable()) {
    DeleteCgiProcess(epoll, fde);
    return;
  }

  if (events & kFdeTimeout) {
    printf("Timeout CGI\n");
    DeleteCgiProcess(epoll, fde);
    return;
  }

  if (events & kFdeWrite) {
    // Write request's body to unisock
    ssize_t write_res = write(cgi_request->GetCgiUnisock(),
                              cgi_process->cgi_input_buffer_.data(),
                              cgi_process->cgi_input_buffer_.size());
    if (write_res < 0) {
      DeleteCgiProcess(epoll, fde);
      return;
    }
    cgi_process->cgi_input_buffer_.EraseHead(write_res);
    if (cgi_process->cgi_input_buffer_.empty()) {
      epoll->Del(fde, kFdeWrite);
    }
  }
  if (events & kFdeRead) {
    // Read data from unisock and store data in buffer
    utils::Byte buf[kDataPerRead];
    ssize_t read_res = read(cgi_request->GetCgiUnisock(), buf, kDataPerRead);
    printf("HandleCgiEvent() read_res == %ld\n", read_res);
    if (read_res < 0) {
      DeleteCgiProcess(epoll, fde);
      return;
    }
    if (read_res == 0) {
      DeleteCgiProcess(epoll, fde);
      return;
    }
    cgi_process->cgi_output_buffer_.AppendDataToBuffer(buf, read_res);
    cgi_response->Parse(cgi_process->cgi_output_buffer_);
  }

  if (events & kFdeError) {
    // Error
    DeleteCgiProcess(epoll, fde);
    return;
  }
}

}  // namespace cgi

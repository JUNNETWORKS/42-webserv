#include "cgi/cgi_process.hpp"

#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils/log.hpp"

namespace cgi {

CgiProcess::CgiProcess(const config::LocationConf *location, Epoll *epoll,
                       ConnSocket *socket)
    : cgi_request_(NULL),
      cgi_response_(NULL),
      cgi_input_buffer_(),
      cgi_output_buffer_(),
      location_(location),
      epoll_(epoll),
      fde_(NULL),
      status_(0),
      socket_(socket) {}

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

http::HttpStatus CgiProcess::RunCgi(server::ConnSocket *conn_sock,
                                    http::HttpRequest &request) {
  SetIsExecuted(true);

  cgi_request_ = new cgi::CgiRequest();
  cgi_response_ = new CgiResponse();
  const http::HttpStatus cgi_res_code =
      cgi_request_->RunCgi(conn_sock, request, *location_);
  if (cgi_res_code != http::OK || cgi_request_->GetCgiUnisock() < 0) {
    delete cgi_request_;
    delete cgi_response_;
    cgi_request_ = NULL;
    cgi_response_ = NULL;
    return cgi_res_code;
  }
  cgi_input_buffer_.insert(cgi_input_buffer_.begin(), request.GetBody().begin(),
                           request.GetBody().end());

  FdEvent *fde =
      CreateFdEvent(cgi_request_->GetCgiUnisock(), HandleCgiEvent, this);
  epoll_->Register(fde);
  if (!cgi_input_buffer_.empty()) {
    epoll_->Add(fde, kFdeWrite);
  } else {
    shutdown(cgi_request_->GetCgiUnisock(), SHUT_WR);
  }
  epoll_->Add(fde, kFdeRead);
  epoll_->SetTimeout(fde, kUnisockTimeout);
  fde_ = fde;
  return http::OK;
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

void CgiProcess::EnableWriteEventToClient() const {
  FdEvent *client_fde = epoll_->GetFdeByFd(socket_->GetFd());
  if (client_fde == NULL) {
    return;
  }
  epoll_->Add(client_fde, kFdeWrite);
}

void CgiProcess::HandleCgiEvent(FdEvent *fde, unsigned int events, void *data,
                                Epoll *epoll) {
  utils::PrintDebugLog("HandleCgiEvent");

  CgiProcess *cgi_process = reinterpret_cast<CgiProcess *>(data);

  if (cgi_process->IsRemovable()) {
    DeleteCgiProcess(epoll, fde);
    return;
  }

  if (events & kFdeTimeout) {
    utils::PrintLog("Timeout CGI");
    DeleteCgiProcess(epoll, fde);
    return;
  }

  bool should_delete_cgi = false;
  if (events & kFdeWrite) {
    should_delete_cgi |= HandleCgiWriteEvent(cgi_process, fde, epoll);
  }
  if (events & kFdeRead) {
    should_delete_cgi |= HandleCgiReadEvent(cgi_process);
  }

  if (should_delete_cgi) {
    // Error
    cgi_process->EnableWriteEventToClient();
    DeleteCgiProcess(epoll, fde);
    return;
  }
}

bool CgiProcess::HandleCgiWriteEvent(CgiProcess *cgi_process, FdEvent *fde,
                                     Epoll *epoll) {
  CgiRequest *cgi_request = cgi_process->cgi_request_;
  // Write request's body to unisock
  ssize_t write_res =
      write(cgi_request->GetCgiUnisock(), cgi_process->cgi_input_buffer_.data(),
            cgi_process->cgi_input_buffer_.size());
  if (write_res < 0) {
    return true;
  }
  cgi_process->cgi_input_buffer_.EraseHead(write_res);
  if (cgi_process->cgi_input_buffer_.empty()) {
    shutdown(cgi_request->GetCgiUnisock(), SHUT_WR);
    epoll->Del(fde, kFdeWrite);
  }
  return false;
}

bool CgiProcess::HandleCgiReadEvent(CgiProcess *cgi_process) {
  CgiRequest *cgi_request = cgi_process->cgi_request_;
  CgiResponse *cgi_response = cgi_process->cgi_response_;
  // Read data from unisock and store data in buffer
  utils::Byte buf[kDataPerRead];
  ssize_t read_res = read(cgi_request->GetCgiUnisock(), buf, kDataPerRead);

  cgi_process->EnableWriteEventToClient();

  if (read_res <= 0) {
    return true;
  }
  cgi_process->cgi_output_buffer_.AppendDataToBuffer(buf, read_res);
  cgi_response->Parse(cgi_process->cgi_output_buffer_);
  return false;
}

}  // namespace cgi

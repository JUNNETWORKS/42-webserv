#include "cgi/cgi_process.hpp"

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

namespace cgi {

CgiProcess::CgiProcess(const config::LocationConf *location, Epoll *epoll)
    : cgi_request_(NULL),
      cgi_response_(NULL),
      cgi_input_buffer_(),
      cgi_output_buffer_(),
      location_(location),
      epoll_(epoll),
      is_executed_(false),
      is_err_(false),
      is_finished_(false) {}

CgiProcess::~CgiProcess() {
  delete cgi_request_;
  delete cgi_response_;
}

Result<void> CgiProcess::RunCgi(http::HttpRequest &request) {
  is_executed_ = true;

  cgi_request_ = AllocateCgiRequest(request);
  cgi_response_ = new CgiResponse();
  if (!cgi_request_->RunCgi() || cgi_request_->GetCgiUnisock() < 0) {
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
  epoll_->Add(fde, kFdeWrite);
  epoll_->Add(fde, kFdeRead);
  return Result<void>();
}

void CgiProcess::KillCgi() {
  kill(cgi_request_->GetPid(), SIGKILL);
}

cgi::CgiRequest *CgiProcess::AllocateCgiRequest(http::HttpRequest &request) {
  std::string cgi_path = location_->GetAbsolutePath(request.GetPath());
  return new cgi::CgiRequest(cgi_path, request, *location_);
}

bool CgiProcess::IsCgiExecuted() const {
  return is_executed_;
}

bool CgiProcess::IsCgiFinished() const {
  return is_finished_;
}

bool CgiProcess::IsUnregistered() const {
  return is_unregistered_;
}

void CgiProcess::SetIsUnregistered(bool is_unregistered) {
  is_unregistered_ = is_unregistered;
}

CgiResponse *CgiProcess::GetCgiResponse() {
  return cgi_response_;
}

void CgiProcess::HandleCgiEvent(FdEvent *fde, unsigned int events, void *data,
                                Epoll *epoll) {
  (void)fde;
  (void)epoll;
  CgiProcess *cgi_process = reinterpret_cast<CgiProcess *>(data);
  CgiRequest *cgi_request = cgi_process->cgi_request_;
  CgiResponse *cgi_response = cgi_process->cgi_response_;

  if (events & kFdeError) {
    // Error
    cgi_process->is_err_ = true;
    return;
  }

  if (events & kFdeWrite) {
    // Write request's body to unisock
    ssize_t write_res = write(cgi_request->GetCgiUnisock(),
                              cgi_process->cgi_input_buffer_.data(),
                              cgi_process->cgi_input_buffer_.size());
    if (write_res < 0) {
      cgi_process->is_err_ = true;
      return;
    }
    cgi_process->cgi_input_buffer_.EraseHead(write_res);
  }
  if (events & kFdeRead) {
    // Read data from unisock and store data in buffer
    utils::Byte buf[kDataPerRead];
    ssize_t read_res = read(cgi_request->GetCgiUnisock(), buf, kDataPerRead);
    if (read_res < 0) {
      cgi_process->is_err_ = true;
      return;
    }
    cgi_process->cgi_output_buffer_.AppendDataToBuffer(buf, read_res);
    cgi_response->Parse(cgi_process->cgi_output_buffer_);
  }
}

}  // namespace cgi

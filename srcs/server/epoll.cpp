#include "server/epoll.hpp"

#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <vector>

#include "result/result.hpp"
#include "utils/error.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

namespace server {

namespace {

epoll_event CalculateEpollEvent(FdEvent *fde) {
  epoll_event epev;
  epev.events = 0;
  if (fde->state & kFdeRead) {
    epev.events |= EPOLLIN;
  }
  if (fde->state & kFdeWrite) {
    epev.events |= EPOLLOUT;
  }
  epev.events |= EPOLLRDHUP;
  epev.data.fd = fde->fd;
  return epev;
}

FdEventEvent CalculateFdEventEvent(FdEvent *fde, epoll_event epev) {
  unsigned int events = 0;
  if ((epev.events & EPOLLIN) && (fde->state & kFdeRead)) {
    events |= kFdeRead;
  }
  if ((epev.events & EPOLLOUT) && (fde->state & kFdeWrite)) {
    events |= kFdeWrite;
  }
  if (epev.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
    if (epev.events & EPOLLERR) {
      utils::PrintDebugLog("CalculateFdEventEvent EPOLLERR %d", fde->fd);
    }
    if (epev.events & EPOLLHUP) {
      utils::PrintDebugLog("CalculateFdEventEvent EPOLLHUP %d", fde->fd);
    }
    if (epev.events & EPOLLRDHUP) {
      utils::PrintDebugLog("CalculateFdEventEvent EPOLLRDHUP %d", fde->fd);
    }
    // EPOLLRDHUP は TCP FIN を受信した場合にフラグが立つが､
    // ネットワークの回線の都合で先に送ったデータよりも後に送った TCP FIN
    // が先に到着する可能性がある｡ これを避けるためには EPOLLRDHUP
    // を受け取った後に read を行い0が返ってくることを確かめる必要がある｡
    //
    // https://ymmt.hatenablog.com/entry/2013/09/05/150116
    events |= kFdeRead | kFdeError;
  }

  FdEventEvent fdee;
  fdee.fde = fde;
  fdee.events = events;
  return fdee;
}

}  // namespace

FdEvent *CreateFdEvent(int fd, FdFunc func, void *data) {
  FdEvent *fde = new FdEvent();
  fde->fd = fd;
  fde->func = func;
  fde->timeout_ms = 0;
  fde->data = data;
  fde->state = 0;
  return fde;
}

void InvokeFdEvent(FdEvent *fde, unsigned int events, Epoll *epoll) {
  fde->func(fde, events, fde->data, epoll);
}

Epoll::Epoll() : epfd_(epoll_create1(EPOLL_CLOEXEC)) {
  if (epfd_ < 0) {
    utils::ErrExit("Epoll constructor");
  }
}

Epoll::~Epoll() {
  close(epfd_);
}

void Epoll::Register(FdEvent *fde) {
  assert(registered_fd_events_.find(fde->fd) == registered_fd_events_.end());
  epoll_event epev = CalculateEpollEvent(fde);

  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fde->fd, &epev) < 0) {
    utils::ErrExit("Epoll::Register epoll_ctl");
  }
  registered_fd_events_[fde->fd] = fde;
}

void Epoll::Unregister(FdEvent *fde) {
  if (registered_fd_events_.find(fde->fd) == registered_fd_events_.end()) {
    return;
  }

  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fde->fd, NULL) < 0) {
    utils::ErrExit("Epoll::Unregister epoll_ctl");
  }
  registered_fd_events_.erase(fde->fd);
}

void Epoll::Set(FdEvent *fde, unsigned int events) {
  unsigned int previous_state = fde->state;
  fde->state = events;
  // 前回と同じだったら epoll_ctl は実行しない
  // kFdeTimeout が変わっても epoll を変更する必要はない
  if ((fde->state & ~kFdeTimeout) == (previous_state & ~kFdeTimeout)) {
    return;
  }

  epoll_event epev = CalculateEpollEvent(fde);
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fde->fd, &epev) < 0) {
    utils::ErrExit("Epoll:Set epoll_ctl");
  }
}

void Epoll::Add(FdEvent *fde, unsigned int events) {
  Set(fde, fde->state | events);
}

void Epoll::Del(FdEvent *fde, unsigned int events) {
  Set(fde, fde->state & ~events);
}

void Epoll::SetTimeout(FdEvent *fde, long timeout_ms) {
  Add(fde, kFdeTimeout);
  fde->timeout_ms = timeout_ms;
  fde->last_active = utils::GetCurrentTimeMs();
}

Result<std::vector<FdEventEvent> > Epoll::WaitEvents(int timeout_ms) {
  std::vector<FdEventEvent> fdee_vec;
  std::vector<epoll_event> epoll_events;
  epoll_events.resize(registered_fd_events_.size());

  int event_num =
      epoll_wait(epfd_, epoll_events.data(), epoll_events.size(), timeout_ms);

  if (event_num < 0) {
    return Error();
  }

  for (int i = 0; i < event_num; ++i) {
    assert(registered_fd_events_.find(epoll_events[i].data.fd) !=
           registered_fd_events_.end());
    FdEvent *fde = registered_fd_events_[epoll_events[i].data.fd];
    FdEventEvent fdee = CalculateFdEventEvent(fde, epoll_events[i]);
    fdee_vec.push_back(fdee);

    fde->last_active = utils::GetCurrentTimeMs();
  }

  return fdee_vec;
}

std::vector<FdEventEvent> Epoll::RetrieveTimeouts() {
  std::vector<FdEventEvent> fdee_vec;

  long current_time = utils::GetCurrentTimeMs();
  for (std::map<int, FdEvent *>::const_iterator it =
           registered_fd_events_.begin();
       it != registered_fd_events_.end(); ++it) {
    FdEvent *fde = it->second;
    if (fde->state & kFdeTimeout &&
        current_time - fde->last_active > fde->timeout_ms) {
      FdEventEvent fdee;
      fdee.fde = fde;
      // TCP FIN が送信したデータより早く来る場合があり､
      // その対策として kFdeError で接続切断をするのではなく､
      // read(conn_fd) の返り値が0(EOF)または-1(Error)だったら切断する｡
      fdee.events = kFdeTimeout | kFdeRead;
      fdee_vec.push_back(fdee);
    }
  }
  return fdee_vec;
}

FdEvent *Epoll::GetFdeByFd(int fd) const {
  if (registered_fd_events_.find(fd) == registered_fd_events_.end()) {
    return NULL;
  }
  return registered_fd_events_.at(fd);
}

}  // namespace server

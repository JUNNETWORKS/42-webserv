#include "server/epoll.hpp"

#include <unistd.h>

#include <cassert>
#include <vector>

#include "result/result.hpp"
#include "utils/error.hpp"

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

}  // namespace

FdEvent *CreateFdEvent(int fd, FdFunc func, void *data) {
  FdEvent *fde = new FdEvent();
  fde->fd = fd;
  fde->func = func;
  fde->data = data;
  return fde;
}

void InvokeFdEvent(FdEvent *fde, unsigned int events, Epoll *epoll) {
  fde->func(fde, events, fde->data, epoll);
}

Epoll::Epoll() : epfd_(epoll_create(1)) {
  if (epfd_ < 0) {
    utils::ErrExit("Epoll constructor");
  }
}

Epoll::~Epoll() {
  close(epfd_);
}

void Epoll::Register(FdEvent *fde) {
  assert(registered_fd_events_.find(fde->fd) == registered_fd_events_.end());
  epoll_event epev;
  epev.events = 0;
  epev.data.fd = fde->fd;

  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fde->fd, &epev) < 0) {
    utils::ErrExit("Epoll::Register epoll_ctl");
  }
  registered_fd_events_[fde->fd] = fde;
}

void Epoll::Unregister(FdEvent *fde) {
  assert(registered_fd_events_.find(fde->fd) != registered_fd_events_.end());
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

    unsigned int events = 0;
    if ((epoll_events[i].events & EPOLLIN) && (fde->state & kFdeRead)) {
      events |= kFdeRead;
    }
    if ((epoll_events[i].events & EPOLLOUT) && (fde->state & kFdeWrite)) {
      events |= kFdeWrite;
    }
    if (epoll_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
      events |= kFdeError;
    }

    FdEventEvent fdee;
    fdee.fde = fde;
    fdee.events = events;
    fdee_vec.push_back(fdee);
  }
  return fdee_vec;
}

}  // namespace server

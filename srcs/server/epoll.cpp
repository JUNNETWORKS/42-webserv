#include "server/epoll.hpp"

#include <unistd.h>

#include <cassert>
#include <vector>

#include "result/result.hpp"
#include "utils/error.hpp"

namespace server {

FdEvent *CreateFdEvent(int fd, FdFunc func, void *data) {
  FdEvent *fde = new FdEvent();
  fde->fd = fd;
  fde->func = func;
  fde->data = data;
  return fde;
}

void InvokeFdEvent(FdEvent *fde, unsigned int events, Epoll *epoll) {
  fde->func(fde->fd, events, fde->data, epoll);
}

Epoll::Epoll() : epfd_(epoll_create(1)) {
  if (epfd_ < 0) {
    utils::ErrExit("Epoll constructor");
  }
}

Epoll::~Epoll() {
  close(epfd_);
}

Result<void> Epoll::AddFd(FdEvent *fd_event, unsigned int events) {
  struct epoll_event epev;
  epev.events = events;
  epev.data.fd = fd_event->fd;

  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_event->fd, &epev) < 0) {
    return Error();
  }
  registered_fd_events_[fd_event->fd] = fd_event;
  return Result<void>();
}

Result<void> Epoll::RemoveFd(int fd) {
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) < 0) {
    return Error();
  }
  FdEvent *fde = registered_fd_events_[fd];
  registered_fd_events_.erase(fd);
  delete fde;
  return Result<void>();
}

Result<void> Epoll::ModifyFd(int fd, unsigned int events) {
  if (registered_fd_events_.find(fd) == registered_fd_events_.end()) {
    return Error();
  }

  struct epoll_event epev;
  epev.events = events;
  epev.data.fd = fd;
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &epev) < 0) {
    return Error();
  }
  return Result<void>();
}

Result<std::vector<FdEventEvent> > Epoll::WaitEvents(int timeout_ms) {
  int maxevents = registered_fd_events_.size();
  std::vector<FdEventEvent> events;
  struct epoll_event *event_arr = new struct epoll_event[maxevents];
  int event_num = epoll_wait(epfd_, event_arr, maxevents, timeout_ms);

  if (event_num <= 0) {
    delete[] event_arr;
    return Error();
  }

  for (int i = 0; i < event_num; ++i) {
    assert(registered_fd_events_.find(event_arr[i].data.fd) !=
           registered_fd_events_.end());
    FdEvent *fd_event = registered_fd_events_[event_arr[i].data.fd];
    FdEventEvent fdee;
    fdee.fde = fd_event;
    fdee.events = event_arr[i].events;
    events.push_back(fdee);
  }
  delete[] event_arr;
  return events;
}

}  // namespace server

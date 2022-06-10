#include "server/epoll.hpp"

#include <unistd.h>

#include <cassert>
#include <vector>

#include "server/fd_event.hpp"
#include "utils/error.hpp"

namespace server {

Epoll::Epoll() : epfd_(epoll_create(1)) {
  if (epfd_ < 0) {
    utils::ErrExit("SocketManager::SocketManager()");
  }
}

Epoll::~Epoll() {
  close(epfd_);
}

bool Epoll::AddFd(const FdEvent &fd_event, unsigned int events) {
  struct epoll_event epev;
  epev.events = events;
  epev.data.fd = fd_event.fd;

  bool is_success = (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_event.fd, &epev) == 0);
  if (is_success) {
    registered_fd_events_[fd_event.fd] = fd_event;
  }
  return is_success;
}

bool Epoll::RemoveFd(int fd) {
  registered_fd_events_.erase(fd);
  return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == 0;
}

bool Epoll::ModifyFd(const FdEvent &fd_event, unsigned int events) {
  if (registered_fd_events_.find(fd_event.fd) == registered_fd_events_.end()) {
    return false;
  }
  registered_fd_events_[fd_event.fd] = fd_event;

  struct epoll_event epev;
  epev.events = events;
  epev.data.fd = fd_event.fd;
  return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd_event.fd, &epev) == 0;
}

bool Epoll::WaitEvents(std::vector<FdEvent> &events, int timeout_ms) {
  int maxevents = registered_fd_events_.size();
  struct epoll_event *event_arr = new struct epoll_event[maxevents];
  int event_num = epoll_wait(epfd_, event_arr, maxevents, timeout_ms);

  if (event_num > 0) {
    for (int i = 0; i < event_num; ++i) {
      assert(registered_fd_events_.find(event_arr[i].data.fd) !=
             registered_fd_events_.end());
      FdEvent fd_event = registered_fd_events_[event_arr[i].data.fd];
      fd_event.events = event_arr[i].events;
      events.push_back(fd_event);
    }
  }
  delete[] event_arr;
  return event_num > 0;
}

}  // namespace server

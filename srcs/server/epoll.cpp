#include "server/epoll.hpp"

#include <unistd.h>

#include <cassert>
#include <vector>

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

bool Epoll::AddFd(int fd, unsigned int events, void *ptr) {
  struct epoll_event epev;
  epev.events = events;
  if (ptr) {
    epev.data.ptr = ptr;
  } else {
    epev.data.fd = fd;
  }
  bool is_success = (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &epev) == 0);
  if (is_success) {
    registered_fd_count_++;
  }
  return is_success;
}

bool Epoll::RemoveFd(int fd) {
  bool is_success = (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == 0);
  if (is_success) {
    registered_fd_count_--;
    assert(registered_fd_count_ >= 0);
  }
  return is_success;
}

bool Epoll::ModifyFd(int fd, unsigned int events, void *ptr) {
  struct epoll_event epev;
  epev.events = events;
  if (ptr) {
    epev.data.ptr = ptr;
  } else {
    epev.data.fd = fd;
  }
  return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &epev) == 0;
}

bool Epoll::WaitEvents(std::vector<struct epoll_event> &events,
                       int timeout_ms) {
  struct epoll_event *event_arr = new struct epoll_event[registered_fd_count_];
  int event_num =
      epoll_wait(epfd_, event_arr, registered_fd_count_, timeout_ms);
  if (event_num > 0) {
    events.insert(events.end(), event_arr, event_arr + event_num);
  }
  delete[] event_arr;
  return event_num > 0;
}

}  // namespace server

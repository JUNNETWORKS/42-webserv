#include "server/epoll.hpp"

#include <unistd.h>

#include <cassert>
#include <vector>

#include "server/file_descriptor.hpp"
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

bool Epoll::AddFd(const FileDescriptor &file_descriptor, unsigned int events) {
  struct epoll_event epev;
  epev.events = events;
  epev.data.fd = file_descriptor.fd;
  bool is_success =
      (epoll_ctl(epfd_, EPOLL_CTL_ADD, file_descriptor.fd, &epev) == 0);
  if (is_success) {
    registered_fds_[file_descriptor.fd] = file_descriptor;
  }
  return is_success;
}

bool Epoll::RemoveFd(int fd) {
  registered_fds_.erase(fd);
  return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == 0;
}

bool Epoll::ModifyFd(const FileDescriptor &file_descriptor,
                     unsigned int events) {
  struct epoll_event epev;
  epev.events = events;
  epev.data.fd = file_descriptor.fd;
  return epoll_ctl(epfd_, EPOLL_CTL_MOD, file_descriptor.fd, &epev) == 0;
}

bool Epoll::WaitEvents(std::vector<const FileDescriptor &> &events,
                       int timeout_ms = -1) {
  int maxevents = registered_fds_.size();
  struct epoll_event *event_arr = new struct epoll_event[maxevents];
  int event_num = epoll_wait(epfd_, event_arr, maxevents, timeout_ms);
  if (event_num > 0) {
    events.insert(events.end(), event_arr, event_arr + event_num);
  }
  delete[] event_arr;
  return event_num > 0;
}

}  // namespace server

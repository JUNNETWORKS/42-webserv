#ifndef SERVER_EPOLL_HPP_
#define SERVER_EPOLL_HPP_

#include <sys/epoll.h>

#include <vector>

namespace server {

class Epoll {
 private:
  const int epfd_;

  int registered_fd_count_;

 public:
  Epoll();
  ~Epoll();

  // 購読するファイルディスクリプタを追加
  // ptr が NULL だった場合は epoll_data.fd に fd がセットされる
  // ptr が 非NULL だった場合は epoll_data.ptr に ptr がセットされる
  bool AddFd(int fd, unsigned long events, void *ptr);

  // 購読しているファイルディスクリプタを削除
  bool RemoveFd(int fd);

  // 購読しているファイルディスクリプタの購読情報を変更
  // ptr が NULL だった場合は epoll_data.fd に fd がセットされる
  // ptr が 非NULL だった場合は epoll_data.ptr に ptr がセットされる
  bool ModifyFd(int fd, unsigned long events, void *ptr);

  // 利用可能なイベントをepoll_waitで取得し､eventsの末尾に挿入する｡
  //
  // timeout は ms 単位｡
  // -1を指定すると1つ以上のイベントが利用可能になるまでブロックする｡
  //
  // イベントが1つ以上 events に追加されたら true を返す｡
  // タイムアウトやエラーの場合は false を返す｡
  bool WaitEvents(std::vector<struct epoll_event> &events, int timeout_ms = -1);

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  Epoll(const Epoll &rhs);
  Epoll &operator=(const Epoll &rhs);
};

}  // namespace server

#endif

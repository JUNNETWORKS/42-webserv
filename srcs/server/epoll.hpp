#ifndef SERVER_EPOLL_HPP_
#define SERVER_EPOLL_HPP_

#include <sys/epoll.h>

#include <map>
#include <vector>

#include "result/result.hpp"
#include "server/fd_event.hpp"

namespace server {
using namespace result;

class Epoll {
 private:
  const int epfd_;

  // map[<fd>] = <FdEvent>
  std::map<int, FdEvent> registered_fd_events_;

 public:
  Epoll();
  ~Epoll();

  // 購読するファイルディスクリプタを追加
  Result<void> AddFd(const FdEvent &fd_event, unsigned int events);

  // 購読しているファイルディスクリプタを削除
  Result<void> RemoveFd(int fd);

  // 購読しているファイルディスクリプタの購読情報を変更
  // fd が Epoll に存在しない場合やエラーの場合は false を返す｡
  Result<void> ModifyFd(const FdEvent &fd_event, unsigned int events);

  // 利用可能なイベントをepoll_waitで取得し､eventsの末尾に挿入する｡
  //
  // timeout は ms 単位｡
  // -1を指定すると1つ以上のイベントが利用可能になるまでブロックする｡
  //
  // イベントが1つ以上 events に追加されたら true を返す｡
  // タイムアウトやエラーの場合は false を返す｡
  Result<std::vector<FdEvent> > WaitEvents(int timeout_ms = -1);

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  Epoll(const Epoll &rhs);
  Epoll &operator=(const Epoll &rhs);
};

}  // namespace server

#endif

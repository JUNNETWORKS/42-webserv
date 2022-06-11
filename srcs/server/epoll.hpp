#ifndef SERVER_EPOLL_HPP_
#define SERVER_EPOLL_HPP_

#include <sys/epoll.h>

#include <map>
#include <vector>

#include "result/result.hpp"
#include "server/fd_event.hpp"

namespace server {
using namespace result;

class Epoll;

// Epoll で利用するイベントハンドラーのインターフェース
typedef void (*FdFunc)(int fd, unsigned int events, void *data, Epoll *epoll);

// Epoll クラスで利用する｡
// fd の種類と発生したイベントの種類を保持する構造体
struct FdEvent {
  int fd;
  FdFunc func;

  // TODO: Timeクラスを作ってタイムアウトを管理する
  // Time timeout;

  void *data;

  // epoll events
  unsigned int events;
};

// Allocate and initialize fdevent
FdEvent *CreateFdEvent(int fd, FdFunc func, void *data);
void InvokeFdEvent(FdEvent *fde, unsigned int events);

class Epoll {
 private:
  const int epfd_;

  // map[<fd>] = <FdEvent>
  std::map<int, FdEvent *> registered_fd_events_;

 public:
  Epoll();
  ~Epoll();

  // 購読するファイルディスクリプタを追加
  Result<void> AddFd(FdEvent *fd_event, unsigned int events);

  // 購読しているファイルディスクリプタを削除
  Result<void> RemoveFd(int fd);

  // 購読しているファイルディスクリプタの購読情報を変更
  Result<void> ModifyFd(FdEvent *fd_event, unsigned int events);

  // 利用可能なイベントをepoll_waitで取得し､eventsの末尾に挿入する｡
  //
  // timeout は ms 単位｡
  // -1を指定すると1つ以上のイベントが利用可能になるまでブロックする｡
  Result<std::vector<FdEvent> > WaitEvents(int timeout_ms = -1);

  // TODO: タイムアウトかどうかを判定し､タイムアウトならばepollから削除する
  // void RemoveTimeoutFds();

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  Epoll(const Epoll &rhs);
  Epoll &operator=(const Epoll &rhs);
};

}  // namespace server

#endif

#ifndef SERVER_EPOLL_HPP_
#define SERVER_EPOLL_HPP_

#include <sys/epoll.h>

#include <map>
#include <vector>

#include "result/result.hpp"

namespace server {
using namespace result;

class Epoll;
struct FdEvent;

// Epoll で利用するイベントハンドラーのインターフェース
typedef void (*FdFunc)(int fd, unsigned int events, void *data, Epoll *epoll);

// Epoll.WaitsEvent() で返す構造体
struct FdEventEvent {
  FdEvent *fde;

  // epoll events
  unsigned int events;
};

// Epoll クラスで利用する｡
struct FdEvent {
  int fd;
  FdFunc func;

  // TODO: Timeクラスを作ってタイムアウトを管理する
  // Time timeout;

  void *data;
};

// FdEvent を動的確保し､引数を元に初期化を行い､それを返す｡
FdEvent *CreateFdEvent(int fd, FdFunc func, void *data);

// fde->func を呼び出す｡
// Epoll のポインタを渡しているのは listen_sock のように func に
void InvokeFdEvent(FdEvent *fde, unsigned int events, Epoll *epoll);

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

  // 購読しているファイルディスクリプタをepollから削除｡
  // 対応する FdEvent を解放｡
  // FdEvent.data は解放しないことに注意｡
  Result<void> RemoveFd(int fd);

  // 購読しているファイルディスクリプタの購読情報を変更
  Result<void> ModifyFd(int fd, unsigned int events);

  // 利用可能なイベントをepoll_waitで取得し､eventsの末尾に挿入する｡
  //
  // timeout は ms 単位｡
  // -1を指定すると1つ以上のイベントが利用可能になるまでブロックする｡
  Result<std::vector<FdEventEvent> > WaitEvents(int timeout_ms = -1);

  // TODO: タイムアウトかどうかを判定し､タイムアウトならばepollから削除する
  // void RemoveTimeoutFds();

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  Epoll(const Epoll &rhs);
  Epoll &operator=(const Epoll &rhs);
};

}  // namespace server

#endif

#ifndef SERVER_EPOLL_HPP_
#define SERVER_EPOLL_HPP_

#include <sys/epoll.h>

#include <ctime>
#include <map>
#include <vector>

#include "result/result.hpp"

namespace server {
using namespace result;

// タイムアウトなどのイベントも通知したいのでイベントを自分で定義する
enum EFdeEvent {
  kFdeRead = 0x0001,
  kFdeWrite = 0x0002,
  kFdeError = 0x0004,
  kFdeTimeout = 0x0008
};

class Epoll;
struct FdEvent;

// Epoll で利用するイベントハンドラーのインターフェース
typedef void (*FdFunc)(FdEvent *fde, unsigned int events, void *data,
                       Epoll *epoll);

// Epoll.WaitsEvent() で返す構造体
struct FdEventEvent {
  FdEvent *fde;

  // EFdeEvent
  unsigned int events;
};

// Epoll クラスで利用する｡
struct FdEvent {
  int fd;
  FdFunc func;

  long timeout_ms;
  long last_active;

  // 監視対象のFdeEvent
  unsigned int state;

  void *data;
};

// FdEvent を動的確保し､引数を元に初期化を行い､それを返す｡
FdEvent *CreateFdEvent(int fd, FdFunc func, void *data);

// fde->func を呼び出す｡
// InvokeFdEvent(FdEventEvent.fde, FdEventEvent.events, &epoll)
//   のように使われる想定
void InvokeFdEvent(FdEvent *fde, unsigned int events, Epoll *epoll);

class Epoll {
 private:
  const int epfd_;

 public:  // TODO : 消す
  // map[<fd>] = <FdEvent>
  std::map<int, FdEvent *> registered_fd_events_;

 public:
  Epoll();
  ~Epoll();

  // Epoll で監視する FdEvent を登録
  void Register(FdEvent *fde);

  // Epoll で監視していた FdEvent を監視対象から外す
  // fde や fde.data の delete はしない｡
  void Unregister(FdEvent *fde);

  // 監視するイベントを変更する
  // events は EFdeEvent である｡ EPOLLIN などではない｡
  void Set(FdEvent *fde, unsigned int events);
  void Add(FdEvent *fde, unsigned int events);
  void Del(FdEvent *fde, unsigned int events);

  void SetTimeout(FdEvent *fde, long timeout_ms);

  // 利用可能なイベントをepoll_waitで取得し､FdEventEventを返す｡
  //
  // timeout は ms 単位｡
  // -1を指定すると1つ以上のイベントが利用可能になるまでブロックする｡
  Result<std::vector<FdEventEvent> > WaitEvents(int timeout_ms = 0);

  // Timeoutなfd取得し､FdEventEventを返す｡
  std::vector<FdEventEvent> RetrieveTimeouts();

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  Epoll(const Epoll &rhs);
  Epoll &operator=(const Epoll &rhs);
};

}  // namespace server

#endif

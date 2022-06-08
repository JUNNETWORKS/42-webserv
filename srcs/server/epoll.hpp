#ifndef SERVER_EPOLL_HPP_
#define SERVER_EPOLL_HPP_

#include <sys/epoll.h>

#include <map>
#include <vector>

#include "server/file_descriptor.hpp"

namespace server {

class Epoll {
 private:
  const int epfd_;

  std::map<int, FileDescriptor> registered_fds_;

 public:
  Epoll();
  ~Epoll();

  // 購読するファイルディスクリプタを追加
  // file_descriptor は Epoll クラス内で保持される｡
  bool AddFd(const FileDescriptor &file_descriptor, unsigned int events);

  // 購読しているファイルディスクリプタを削除
  bool RemoveFd(int fd);

  // 購読しているファイルディスクリプタの購読情報を変更
  // file_descriptor は Epoll クラス内で保持される｡
  // fd が Epoll に存在しない場合やエラーの場合は false を返す｡
  bool ModifyFd(const FileDescriptor &file_descriptor, unsigned int events);

  // 利用可能なイベントをepoll_waitで取得し､eventsの末尾に挿入する｡
  //
  // timeout は ms 単位｡
  // -1を指定すると1つ以上のイベントが利用可能になるまでブロックする｡
  //
  // イベントが1つ以上 events に追加されたら true を返す｡
  // タイムアウトやエラーの場合は false を返す｡
  bool WaitEvents(std::vector<const FileDescriptor &> &events,
                  int timeout_ms = -1);

 private:
  // epoll instance が片方のみでcloseされるのを防ぐためコピー操作は禁止
  Epoll(const Epoll &rhs);
  Epoll &operator=(const Epoll &rhs);
};

}  // namespace server

#endif

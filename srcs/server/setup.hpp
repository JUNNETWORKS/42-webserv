#ifndef SERVER_SETUP_HPP_
#define SERVER_SETUP_HPP_

#include <vector>

#include "config/config.hpp"
#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/types.hpp"

namespace server {
using namespace result;

Result<void> RegisterListenSockets(Epoll &epoll, const config::Config &config);

// config内のバーチャルサーバの情報を元に必要なソケットをオープンし､リッスン状態にする｡
// 返り値は map[<listen_fd>] = <port> の形のmap
Result<ListenFdPortMap> OpenLilstenFds(const config::Config &config);

// fdsのすべての要素に対してcloseシステムコールを実行する｡
void CloseAllFds(const ListenFdPortMap &listen_fd_port_map);

// listen_fd_port_map を Epoll で監視するようにする
void AddListenFds2Epoll(Epoll &epoll, const config::Config &config,
                        const ListenFdPortMap &listen_fd_port_map);

}  // namespace server
#endif

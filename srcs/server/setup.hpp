#ifndef SERVER_SETUP_HPP_
#define SERVER_SETUP_HPP_

#include <vector>

#include "config/config.hpp"
#include "server/types.hpp"

namespace server {

// config内のバーチャルサーバの情報を元に必要なソケットをオープンし､リッスン状態にする｡
// 返り値は map[<listen_fd>] = <port> の形のmap
bool OpenLilstenFds(ListenFdPortMap &listen_fd_port_map,
                    const config::Config &config);

// fdsのすべての要素に対してcloseシステムコールを実行する｡
void CloseAllFds(const ListenFdPortMap &listen_fd_port_map);

}  // namespace server
#endif

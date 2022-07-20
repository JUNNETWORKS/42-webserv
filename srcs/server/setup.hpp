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

}  // namespace server
#endif

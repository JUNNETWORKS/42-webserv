#include "config/config.hpp"
#include "server/epoll.hpp"
#include "server/event_loop.hpp"
#include "server/setup.hpp"
#include "server/socket_manager.hpp"
#include "server/types.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

int main(int argc, char const *argv[]) {
  setbuf(stdout, NULL);
  config::Config config;
  if (argc >= 2) {
    try {
      config = config::ParseConfig(argv[1]);
    } catch (const std::exception &err) {
      std::cerr << "Parser Error!!\n";
      std::cerr << "Error message: " << err.what() << std::endl;
      exit(EXIT_FAILURE);
    }
  } else {
    config = config::CreateSampleConfig();
  }
  config.Print();

  server::ListenFdPortMap listen_fd_port_map;
  if (!server::OpenLilstenFds(listen_fd_port_map, config)) {
    utils::ErrExit("OpenListenFds()");
  }

  // epoll インスタンス作成
  server::Epoll epoll;
  server::SocketManager socket_manager(epoll);
  socket_manager.AppendListenFd(listen_fd_port_map);

  server::StartEventLoop(epoll, socket_manager, config);

  return 0;
}

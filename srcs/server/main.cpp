#include "config/config.hpp"
#include "result/result.hpp"
#include "server/epoll.hpp"
#include "server/event_loop.hpp"
#include "server/setup.hpp"
#include "server/socket.hpp"
#include "server/socket_event_handler.hpp"
#include "server/types.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

int main(int argc, char const *argv[]) {
  using namespace result;

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

  Result<server::ListenFdPortMap> result = server::OpenLilstenFds(config);
  if (result.IsErr()) {
    utils::ErrExit("server::OpenLilstenFds()");
  }

  server::ListenFdPortMap listen_fd_port_map = result.Ok();

  // epoll インスタンス作成
  server::Epoll epoll;

  for (server::ListenFdPortMap::const_iterator it = listen_fd_port_map.begin();
       it != listen_fd_port_map.end(); ++it) {
    int listen_fd = it->first;
    std::string port = it->second;
    server::Socket *listen_sock =
        new server::Socket(listen_fd, server::Socket::ListenSock, port, config);
    server::FdEvent *fde = server::CreateFdEvent(
        listen_fd, server::HandleListenSocketEvent, listen_sock);
    if (epoll.AddFd(fde, EPOLLIN).IsErr()) {
      utils::ErrExit("Epoll.AddFd()");
    }
  }

  server::StartEventLoop(epoll);

  return 0;
}

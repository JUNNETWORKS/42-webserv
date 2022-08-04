#include <csignal>

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

namespace {
const std::string kDefaultConfigPath = "configurations/default.conf";
}

int main(int argc, char const *argv[]) {
  using namespace result;

  // 多くのアプリケーションではSIGPIPEを無視し､write() の返り値で判定する
  // https://stackoverflow.com/questions/3469567/broken-pipe-error
  signal(SIGPIPE, SIG_IGN);

  setbuf(stdout, NULL);
  std::string config_path;
  if (argc == 2) {
    config_path = argv[1];
  } else if (argc == 1) {
    config_path = kDefaultConfigPath;
  } else {
    std::cerr << "Error: Too many arguments." << std::endl;
    exit(EXIT_FAILURE);
  }

  config::Config config;
  try {
    config = config::ParseConfig(config_path);
  } catch (const std::exception &err) {
    std::cerr << "Parser Error!!\n";
    std::cerr << "Error message: " << err.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  if (!config.IsValid()) {
    std::cerr << "Config is invalid!!" << std::endl;
    exit(EXIT_FAILURE);
  }
  config.Print();

  // epoll インスタンス作成
  server::Epoll epoll;

  // listen socket を作成
  if (RegisterListenSockets(epoll, config).IsErr()) {
    utils::ErrExit("server::RegisterListenSockets()");
  }

  server::StartEventLoop(epoll);

  return 0;
}

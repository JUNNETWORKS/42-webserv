#include <signal.h>

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

  // 多くのアプリケーションではSIGPIPEを無視し､write() の返り値で判定する
  // https://stackoverflow.com/questions/3469567/broken-pipe-error
  signal(SIGPIPE, SIG_IGN);

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

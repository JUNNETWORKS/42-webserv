#include "config/config.hpp"
#include "server/event_loop.hpp"
#include "server/setup.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

int main(int argc, char const *argv[]) {
  config::Config config;
  if (argc >= 2) {
    config = config::ParseConfig(argv[1]);
  } else {
    config = config::CreateSampleConfig();
  }

  // Setup configuration
  config.Print();

  // listen_fd を作成
  std::vector<int> listen_fds = server::OpenLilstenFds(config);

  server::StartEventLoop(listen_fds, config);

  return 0;
}

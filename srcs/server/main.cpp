#include "config/config.hpp"
#include "server/event_loop.hpp"
#include "server/setup.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    utils::ErrExit("%s <config_path>\n", argv[0]);
  }

  // Setup configuration
  // config::Config config = config::parseConfig(argv[1]);
  config::Config config = config::CreateSampleConfig();

  // listen_fd を作成
  std::vector<int> listen_fds = server::OpenLilstenFds(config);

  server::StartEventLoop(listen_fds, config);

  return 0;
}

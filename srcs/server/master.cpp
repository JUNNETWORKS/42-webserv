#include "config/config.hpp"
#include "server/event_loop.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    utils::ErrExit("%s <config_path>\n", argv[0]);
  }

  // Setup configuration
  // config::Config config = config::parseConfig(argv[1]);
  // config::Config *config = config::GetSampleConfig();

  // listen_fd を作成
  int listen_fd = utils::inetListen("8080", 10, NULL);
  std::cout << "Listen at 127.0.0.1:8080" << std::endl;

  server::StartEventLoop(listen_fd);

  return 0;
}

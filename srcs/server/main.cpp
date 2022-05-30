#include "config/config.hpp"
#include "server/event_loop.hpp"
#include "server/setup.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

int main(int argc, char const *argv[]) {
  config::Config config;
  if (argc >= 2) {
    try {
      config = config::ParseConfig(argv[1]);
    } catch (const std::exception &err) {
      std::cerr << "Parser Error!!\n";
      std::cerr << "Error message: " << err.what() << std::cout;
      exit(EXIT_FAILURE);
    }
  } else {
    config = config::CreateSampleConfig();
  }
  config.Print();

  std::vector<int> listen_fds = server::OpenLilstenFds(config);

  server::StartEventLoop(listen_fds, config);

  return 0;
}

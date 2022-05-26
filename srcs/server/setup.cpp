#include "server/setup.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "utils/inet_sockets.hpp"

namespace server {

void CloseAllFds(const std::vector<int> &fds) {
  for (std::vector<int>::const_iterator it = fds.begin(); it != fds.end();
       ++it) {
    close(*it);
  }
}

std::vector<int> OpenLilstenFds(const config::Config &config) {
  std::set<config::PortType> used_ports;
  std::vector<int> fds;

  const config::Config::VirtualServerConfVector &virtual_servers =
      config.GetVirtualServerConfs();
  for (config::Config::VirtualServerConfVector::const_iterator it =
           virtual_servers.begin();
       it != virtual_servers.end(); ++it) {
    if (used_ports.find(it->GetListenPort()) != used_ports.end()) {
      // The port has already binded
      continue;
    }
    std::cout << "Open " << it->GetListenPort() << std::endl;
    int fd = utils::InetListen(it->GetListenPort().c_str(), SOMAXCONN, NULL);
    if (fd == -1) {
      CloseAllFds(fds);
      throw std::exception();
    }
    fds.push_back(fd);
    used_ports.insert(it->GetListenPort());
  }
  return fds;
}

};  // namespace server

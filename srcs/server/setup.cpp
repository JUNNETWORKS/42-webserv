#include "server/setup.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "utils/inet_sockets.hpp"

namespace server {

void CloseAllFds(const ListenFdPortMap &listen_fd_port_map) {
  for (ListenFdPortMap::const_iterator it = listen_fd_port_map.begin();
       it != listen_fd_port_map.end(); ++it) {
    close(it->first);
  }
}

bool OpenLilstenFds(ListenFdPortMap &listen_fd_port_map,
                    const config::Config &config) {
  std::set<config::PortType> used_ports;

  const config::Config::VirtualServerConfVector &virtual_servers =
      config.GetVirtualServerConfs();
  for (config::Config::VirtualServerConfVector::const_iterator it =
           virtual_servers.begin();
       it != virtual_servers.end(); ++it) {
    if (used_ports.find(it->GetListenPort()) != used_ports.end()) {
      // The port has already binded
      continue;
    }
    int fd = utils::InetListen(it->GetListenPort().c_str(), SOMAXCONN, NULL);
    if (fd == -1) {
      CloseAllFds(listen_fd_port_map);
      return false;
    }
    listen_fd_port_map[fd] = it->GetListenPort();
    used_ports.insert(it->GetListenPort());
  }
  return true;
}

}  // namespace server

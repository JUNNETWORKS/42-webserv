#include "server/setup.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "utils/inet_sockets.hpp"

namespace server {

ServerException::ServerException(const char *errmsg) : errmsg_(errmsg) {}

const char *ServerException::what() const throw() {
  return errmsg_;
}

void CloseAllFds(const ListenFdPortMap &fds) {
  for (ListenFdPortMap::const_iterator it = fds.begin(); it != fds.end();
       ++it) {
    close(it->first);
  }
}

ListenFdPortMap OpenLilstenFds(const config::Config &config) {
  std::set<config::PortType> used_ports;
  ListenFdPortMap listen_fd_port_map;

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
      throw ServerException("OpenListenFds: failed to create socket.");
    }
    listen_fd_port_map[fd] = it->GetListenPort();
    used_ports.insert(it->GetListenPort());
  }
  return listen_fd_port_map;
}

}  // namespace server

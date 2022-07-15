#include "server/setup.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "config/config.hpp"
#include "server/epoll.hpp"
#include "server/socket.hpp"
#include "server/socket_event_handler.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {

Result<void> RegisterListenSockets(Epoll &epoll, const config::Config &config) {
  Result<server::ListenFdPortMap> result = OpenLilstenFds(config);
  if (result.IsErr()) {
    return Error();
  }
  AddListenFds2Epoll(epoll, config, result.Ok());
  return Result<void>();
}

Result<ListenFdPortMap> OpenLilstenFds(const config::Config &config) {
  ListenFdPortMap listen_fd_port_map;
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
      return Error("OpenLilstenFds");
    }
    listen_fd_port_map[fd] = it->GetListenPort();
    used_ports.insert(it->GetListenPort());
  }
  return listen_fd_port_map;
}

void CloseAllFds(const ListenFdPortMap &listen_fd_port_map) {
  for (ListenFdPortMap::const_iterator it = listen_fd_port_map.begin();
       it != listen_fd_port_map.end(); ++it) {
    close(it->first);
  }
}

void AddListenFds2Epoll(Epoll &epoll, const config::Config &config,
                        const ListenFdPortMap &listen_fd_port_map) {
  for (ListenFdPortMap::const_iterator it = listen_fd_port_map.begin();
       it != listen_fd_port_map.end(); ++it) {
    int listen_fd = it->first;
    std::string port = it->second;
    // TODO: sockaddr_storage を渡せるようにする
    ListenSocket *listen_sock = new ListenSocket(listen_fd, config);
    FdEvent *fde =
        CreateFdEvent(listen_fd, HandleListenSocketEvent, listen_sock);
    epoll.Register(fde);
    epoll.Add(fde, kFdeRead);
  }
}

}  // namespace server

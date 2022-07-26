#include "server/setup.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "config/config.hpp"
#include "server/epoll.hpp"
#include "server/setup.hpp"
#include "server/socket.hpp"
#include "server/socket_event_handler.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {

namespace {
// fdsのすべての要素に対してcloseシステムコールを実行する｡
void CloseAllFds(const std::vector<int> &listen_fds);
}  // namespace

Result<void> RegisterListenSockets(Epoll &epoll, const config::Config &config) {
  std::set<config::PortType> used_ip_ports;
  std::vector<int> fds;
  const config::Config::VirtualServerConfVector &virtual_servers =
      config.GetVirtualServerConfs();
  for (config::Config::VirtualServerConfVector::const_iterator it =
           virtual_servers.begin();
       it != virtual_servers.end(); ++it) {
    std::string ip_port = it->GetListenIp() + ":" + it->GetListenPort();
    std::string port_any_ip = "0.0.0.0:" + it->GetListenPort();
    if (used_ip_ports.find(ip_port) != used_ip_ports.end() ||
        used_ip_ports.find(port_any_ip) != used_ip_ports.end()) {
      // The port has already binded
      continue;
    }

    SocketAddress socket_address;
    Result<int> listen_res = utils::InetListen(
        it->GetListenIp(), it->GetListenPort(), SOMAXCONN, &socket_address);
    if (listen_res.IsErr()) {
      CloseAllFds(fds);
      return Error("RegisterListenSockets");
    }
    int fd = listen_res.Ok();

    ListenSocket *listen_sock = new ListenSocket(fd, socket_address, config);
    FdEvent *fde = CreateFdEvent(fd, HandleListenSocketEvent, listen_sock);
    epoll.Register(fde);
    epoll.Add(fde, kFdeRead);

    used_ip_ports.insert(ip_port);
    fds.push_back(fd);
  }
  return Result<void>();
}

namespace {
void CloseAllFds(const std::vector<int> &listen_fds) {
  for (std::vector<int>::const_iterator it = listen_fds.begin();
       it != listen_fds.end(); ++it) {
    close(*it);
  }
}
}  // namespace

}  // namespace server

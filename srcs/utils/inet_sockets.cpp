/* To get definitions of NI_MAXHOST and NI_MAXSERV from <netdb.h> */
#define _DEFAULT_SOURCE

#include "inet_sockets.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

#include "server/socket_address.hpp"

namespace utils {

int InetConnect(const std::string &host, const std::string &service, int type) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
  hints.ai_socktype = type;

  s = getaddrinfo(host.c_str(), service.c_str(), &hints, &result);
  if (s != 0) {
    errno = ENOSYS;
    return -1;
  }

  /* Walk through returned list until we find an address structure
  that can be used to successfully connect a socket */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK,
                 rp->ai_protocol);
    if (sfd == -1)
      continue; /* On error, try next address */

    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break; /* Success */

    /* Connect failed: close this socket and try next addresss */
    close(sfd);
  }
  freeaddrinfo(result);
  return (rp == NULL) ? -1 : sfd;
}

/* Public interfaces: InetBind() and InetListen() */
static int InetPassiveSocket(const std::string &service, int type,
                             server::SocketAddress *sockaddr, bool doListen,
                             int backlog) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, optval, s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = type;
  hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
  hints.ai_flags = AI_PASSIVE; /* Use wildcadrd IP address */

  s = getaddrinfo(NULL, service.c_str(), &hints, &result);
  if (s != 0)
    return -1;

  /* Walk through returned list until we find an address structure
  that can be used to successfully create and bind a socket */
  optval = 1;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK,
                 rp->ai_protocol);
    if (sfd == -1)
      continue; /* On error, try next address */

    if (doListen) {
      if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==
          -1) {
        close(sfd);
        freeaddrinfo(result);
        return -1;
      }
    }

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; /* Success */

    /* bind() failed: close this socket and try next address */
    close(sfd);
  }

  if (rp != NULL && doListen) {
    if (listen(sfd, backlog) == -1) {
      freeaddrinfo(result);
      return -1;
    }
  }
  if (rp != NULL && sockaddr != NULL) {
    sockaddr->SetSockaddr(rp->ai_addr, rp->ai_addrlen);
  }
  freeaddrinfo(result);
  return (rp == NULL) ? -1 : sfd;
}

Result<int> InetListen(const std::string &service, int backlog,
                       server::SocketAddress *sockaddr) {
  int fd = InetPassiveSocket(service, SOCK_STREAM, sockaddr, true, backlog);
  if (fd < 0) {
    return Error();
  }
  return fd;
}

Result<int> InetBind(const std::string &service, int type,
                     server::SocketAddress *sockaddr) {
  int fd = InetPassiveSocket(service, type, sockaddr, false, 0);
  if (fd < 0) {
    return Error();
  }
  return fd;
}

std::string InetAddressStr(const struct sockaddr *addr, socklen_t addrlen) {
  std::stringstream ss;
  char host[NI_MAXHOST], service[NI_MAXSERV];

  if (getnameinfo(addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV,
                  NI_NUMERICSERV) == 0) {
    ss << "(" << host << ", " << service << ")";
  } else {
    ss << "(?UNKNOWN?)";
  };
  return ss.str();
}

void LogConnectionInfoToStdout(struct sockaddr_storage &client_addr) {
  socklen_t len = sizeof(struct sockaddr_storage);
  std::string addr_str = InetAddressStr((struct sockaddr *)&client_addr, len);
  std::cout << "Connection from " << addr_str << std::endl;
}

}  // namespace utils

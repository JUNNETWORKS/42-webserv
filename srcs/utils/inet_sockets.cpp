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

#include <string>

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
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
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
                             socklen_t *addrlen, bool doListen, int backlog) {
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
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
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
  if (rp != NULL && addrlen != NULL)
    *addrlen = rp->ai_addrlen; /* Return address structure size */
  freeaddrinfo(result);
  return (rp == NULL) ? -1 : sfd;
}

// TODO: Result を返すようにする
int InetListen(const std::string &service, int backlog, socklen_t *addrlen) {
  return InetPassiveSocket(service, SOCK_STREAM, addrlen, true, backlog);
}

int InetBind(const std::string &service, int type, socklen_t *addrlen) {
  return InetPassiveSocket(service, type, addrlen, false, 0);
}

std::string GetSockaddrPort(const struct sockaddr_storage &addr) {
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  char service[NI_MAXSERV];
  if (getnameinfo((const struct sockaddr *)&addr, addrlen, NULL, 0, service,
                  NI_MAXSERV, NI_NUMERICSERV) != 0) {
    return "";
  }
  return service;
}

std::string GetSockaddrIp(const struct sockaddr_storage &addr) {
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  char host[NI_MAXHOST];
  if (getnameinfo((const struct sockaddr *)&addr, addrlen, host, NI_MAXHOST,
                  NULL, 0, NI_NUMERICHOST) != 0) {
    return "";
  }
  return host;
}

std::string GetSockaddrName(const struct sockaddr_storage &addr) {
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  char host[NI_MAXHOST];
  if (getnameinfo((const struct sockaddr *)&addr, addrlen, host, NI_MAXHOST,
                  NULL, 0, NI_NAMEREQD) != 0) {
    return "";
  }
  return host;
}

char *InetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                     char *addrStr, int addrStrLen) {
  char host[NI_MAXHOST], service[NI_MAXSERV];

  if (getnameinfo(addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV,
                  NI_NUMERICSERV) == 0)
    snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
  else
    snprintf(addrStr, addrStrLen, "(?UNKNOWN?)");
  return addrStr;
}

void LogConnectionInfoToStdout(struct sockaddr_storage &client_addr) {
  socklen_t len = sizeof(struct sockaddr_storage);
  char addrStr[utils::IS_ADDR_STR_LEN];
  InetAddressStr((struct sockaddr *)&client_addr, len, addrStr,
                 utils::IS_ADDR_STR_LEN);
  printf("Connection from %s\n", addrStr);
}

}  // namespace utils

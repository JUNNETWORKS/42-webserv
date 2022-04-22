/* To get definitions of NI_MAXHOST and NI_MAXSERV from <netdb.h> */
#define _DEFAULT_SOURCE

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "read_line.hpp"
#include "tcp.hpp"
#include "utils.hpp"

#define BACKLOG 50

int main(int argc, char const *argv[]) {
  uint32_t seqNum;
  char reqLenStr[INT_LEN]; /* Length of requested sequence */
  char seqNumStr[INT_LEN]; /* Start of granted sequence */
  struct sockaddr_storage
      claddr; /* sockaddr_storage は受信時にバイト数が不明の時に使う.
                IPv4, IPv6 両方対応 */
  int lfd, cfd, optval, reqLen;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
  char addrStr[ADDRSTRLEN];
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];

  if (argc > 1 && strcmp(argv[1], "--help") == 0)
    usageErr("%s [init-seq-num]\n", argv[0]);

  seqNum = (argc > 1) ? atoi(argv[1]) : 0;
  if (signal(SIGPIPE, SIG_IGN) ==
      SIG_ERR)  // writeした時に発生する恐れのあるSIGPIPEを無視
    fatal("signal");

  /* Call getaddrinfo() to obtain a list of addresses that we can try binding to
   */

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC; /* Allow IPv4 and IPv6 */
  hints.ai_flags =
      AI_PASSIVE |
      AI_NUMERICSERV; /* Wildcard IP address; service name is numeric */

  if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0)
    fatal("getaddrinfo");

  /* Walk through returned list until we find an address structure
  that can be used to successfully create and bind a socket */
  optval = 1;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (lfd == -1)
      continue;
    // SO_REUSEADDR
    // を使うことでTCP本来の仕様に近づく。ほとんどのTCPサーバーはこのオプションを有効にしている。
    // 多くのUNIX系OSではlistenしていないソケットがポートを使っている場合、そのポートは別のプログラムが使えない。
    // しかし、ソケットの識別は (local IP, local port, remote ip, remote port)
    //   で識別され、TCPの仕様的にはOK.
    // SO_REUSEADDRオプションを付けることでlistenしていないポートが対象のポートを既に使っていても、使えるようになる。
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==
        -1)
      fatal("setsockopt");

    if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
      break; /* success */

    /* bind() failed: close this socket and try next address */
    close(lfd);
  }

  if (rp == NULL)
    errExit("Could not bind socket to any address");

  if (listen(lfd, BACKLOG) == -1)
    fatal("listen");

  freeaddrinfo(result);

  while (true) { /* Handle clients iteratively */
    /* Accept a client connection, obtaining client's address */

    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(lfd, (struct sockaddr *)&claddr, &addrlen);
    if (cfd == -1)
      errExit("accept");

    if (getnameinfo((struct sockaddr *)&claddr, addrlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, 0) == 0) {
      snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
    } else {
      snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN?)");
    }
    printf("Connection from %s\n", addrStr);

    /* Read client request, send sequence number back */

    if (readLine(cfd, reqLenStr, INT_LEN) <= 0) {
      close(cfd);
      continue; /* failed read; skip request */
    }

    reqLen = atoi(reqLenStr);
    if (reqLen <= 0) { /* Watch for misbehaving clients */
      close(cfd);
      continue; /* Bad request: skip it */
    }

    snprintf(seqNumStr, INT_LEN, "%d\n", seqNum);
    if (write(cfd, &seqNumStr, strlen(seqNumStr)) != strlen(seqNumStr))
      errExit("Error on write");

    seqNum += reqLen;     /* Update sequence number */
    if (close(cfd) == -1) /* Close connection */
      fatal("close");
  }
}

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

int main(int argc, char const *argv[]) {
  const char *reqLenStr;   /* Requested length of sequence */
  char seqNumStr[INT_LEN]; /* Start of granted sequence */
  int cfd;
  ssize_t numRead;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageErr("%s server-host [sequence-len]\n", argv[0]);

  /* Call getaddrinfo() to obtain a list of addresses that we can try connecting
    to */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;

  if (getaddrinfo(argv[1], PORT_NUM, &hints, &result) != 0)
    errExit("getaddrinfo\n");

  /* Walk throuhgh returned list until we find an address structure that can be
    used to successfully connect a socket */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (cfd == -1)
      continue; /* On error, try next address */

    if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break; /* Success */

    /* Connect failed: close this socket and try next address */
    close(cfd);
  }

  if (rp == NULL)
    fatal("Could not connect socket to any address\n");

  freeaddrinfo(result);

  /* Send requested sequence length, with terminating newline */
  reqLenStr = (argc > 2) ? argv[2] : "1";
  if (write(cfd, reqLenStr, strlen(reqLenStr)) != strlen(reqLenStr))
    fatal("Partial/failed write (reqLenStr)");
  if (write(cfd, "\n", 1) != 1)
    fatal("Partial/failed write (newline)");

  /* Read and display sequence number returned by server */
  numRead = readLine(cfd, seqNumStr, INT_LEN);
  if (numRead == -1)
    fatal("readLine\n");
  if (numRead == -1)
    fatal("Unexpected EOF from server\n");

  printf("Sequence number: %s", seqNumStr); /* Includes '\n' */

  return 0;
}

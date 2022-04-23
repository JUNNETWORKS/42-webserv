#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "inet_sockets.hpp"
#include "udp_iterative_echo.hpp"
#include "utils.hpp"

int main(int argc, char const *argv[]) {
  int sfd, j;
  size_t len;
  ssize_t numRead;
  char buf[BUF_SIZE];

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageErr("%s: host msg...\n", argv[0]);

  /* Construct server address from first command-line argument */
  sfd = inetConnect(argv[1], SERVICE, SOCK_DGRAM);
  if (sfd == -1)
    fatal("Could not connect to server socket");

  /* Send remaining command-line arguments to server as separate datagrams */
  for (j = 2; j < argc; j++) {
    len = strlen(argv[j]);
    if (write(sfd, argv[j], len) != len)
      fatal("partial/failed write");

    numRead = read(sfd, buf, BUF_SIZE);
    if (numRead == -1)
      errExit("read");

    printf("[%ld bytes] %.*s\n", (long)numRead, (int)numRead, buf);
  }
  return 0;
}

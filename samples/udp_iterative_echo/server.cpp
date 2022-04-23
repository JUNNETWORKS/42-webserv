#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "inet_sockets.hpp"
#include "udp_iterative_echo.hpp"
#include "utils.hpp"

int main(int argc, char const *argv[]) {
  int sfd;
  ssize_t numRead;
  socklen_t addrlen, len;
  struct sockaddr_storage claddr;
  char buf[BUF_SIZE];
  char addrStr[IS_ADDR_STR_LEN];

  sfd = inetBind(SERVICE, SOCK_DGRAM, &addrlen);
  if (sfd == -1)
    errExit("Could not create server socket (%s)", strerror(errno));

  /* Receive datagrams and return copies to senders */
  while (true) {
    len = sizeof(struct sockaddr_storage);
    numRead = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *)&claddr, &len);
    if (numRead == -1)
      errExit("recvfrom");

    if (sendto(sfd, buf, numRead, 0, (struct sockaddr *)&claddr, len) !=
        numRead)
      errExit("Error echoing response to %s (%s)",
              inetAddressStr((struct sockaddr *)&claddr, len, addrStr,
                             IS_ADDR_STR_LEN),
              strerror(errno));
  }

  return 0;
}

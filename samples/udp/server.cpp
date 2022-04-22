#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "udp.hpp"
#include "utils.hpp"

int main(int argc, char const *argv[]) {
  // 新規ソケットの作成
  int socket_fd;
  if (!(socket_fd = socket(AF_INET, SOCK_DGRAM, 0))) {
    fprintf(stderr, "socket() failed!\n");
    exit(1);
  }

  // well-knownアドレスにバインド
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT_NUM);

  if (bind(socket_fd, (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr_in)) == -1)
    fatal("bind() failed!\n");
  char addr_str[INET_ADDRSTRLEN];
  if (inet_ntop(server_addr.sin_family, &server_addr.sin_addr, addr_str,
                INET_ADDRSTRLEN) == 0)
    fatal("failed to get server address\n");
  printf("Server waiting a request at %s:%u\n", addr_str,
         ntohs(server_addr.sin_port));

  // Receive messages, convert to uppercase, and return to client
  while (true) {
    struct sockaddr_in client_addr;
    char buffer[BUF_SIZE];

    socklen_t len = sizeof(struct sockaddr_in);
    ssize_t numBytes = recvfrom(socket_fd, buffer, BUF_SIZE, 0,
                                (struct sockaddr *)&client_addr, &len);

    if (numBytes == -1)
      fatal("recvfrom() failed!\n");

    char client_addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str,
                  INET_ADDRSTRLEN) == NULL) {
      fatal("inet_ntop() failed!\n");
    } else {
      printf("Server received %ld bytes from (%s, %u)\n", numBytes,
             client_addr_str, ntohs(client_addr.sin_port));
    }

    for (int i = 0; i < numBytes; i++) {
      buffer[i] = toupper(buffer[i]);
    }

    if (sendto(socket_fd, buffer, numBytes, 0, (struct sockaddr *)&client_addr,
               sizeof(struct sockaddr_in)) != numBytes)
      fatal("failed to send messge to client\n");
  }

  return 0;
}

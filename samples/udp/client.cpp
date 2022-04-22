#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp.hpp"
#include "utils.hpp"

int main(int argc, char const *argv[]) {
  if (argc < 3 || strcmp(argv[1], "--help") == 0)
    usageErr("%s host-address msg...\n", argv[0]);

  // 新規ソケットの作成
  int socket_fd;
  if (!(socket_fd = socket(AF_INET, SOCK_DGRAM, 0)))
    fatal("socket() failed!\n");

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT_NUM);
  if (inet_pton(server_addr.sin_family, argv[1], &server_addr.sin_addr) <= 0)
    fatal("inet_pton faield for address '%s'", argv[1]);
  char addr_str[INET_ADDRSTRLEN];
  if (inet_ntop(server_addr.sin_family, &server_addr.sin_addr, addr_str,
                INET_ADDRSTRLEN) == 0)
    fatal("failed to get server address\n");
  printf("The client will send messages to %s:%u\n", addr_str,
         ntohs(server_addr.sin_port));

  /* Send messages to server; echo responses on stdout */
  for (int i = 2; i < argc; i++) {
    int msgLen = strlen(argv[i]);
    if (sendto(socket_fd, argv[i], msgLen, 0, (struct sockaddr *)&server_addr,
               sizeof(struct sockaddr_in)) != msgLen)
      fatal("sendto() failed\n");

    char res_buf[BUF_SIZE];
    ssize_t numBytes = recvfrom(socket_fd, res_buf, BUF_SIZE, 0, NULL, NULL);
    if (numBytes == -1)
      fatal("recvfrom() failed\n");

    printf("Response %d: %.*s\n", i - 1, (int)numBytes, res_buf);
  }

  return 0;
}

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 10
#define PORT_NUM 50002

int main(int argc, char const *argv[]) {
  if (argc < 3 || strcmp(argv[1], "--help") == 0) {
    printf("%s host-address msg...\n", argv[0]);
    exit(1);
  }

  // 新規ソケットの作成
  int socket_fd;
  if (!(socket_fd = socket(AF_INET, SOCK_DGRAM, 0))) {
    fprintf(stderr, "socket() failed!\n");
    exit(1);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT_NUM);
  if (inet_pton(server_addr.sin_family, argv[1], &server_addr.sin_addr) <= 0) {
    fprintf(stderr, "inet_pton faield for address '%s'", argv[1]);
    exit(1);
  }
  char addr_str[INET_ADDRSTRLEN];
  if (inet_ntop(server_addr.sin_family, &server_addr.sin_addr, addr_str,
                INET_ADDRSTRLEN) == 0) {
    fprintf(stderr, "failed to get server address\n");
    exit(1);
  }
  printf("The client will send messages to %s:%u\n", addr_str,
         ntohs(server_addr.sin_port));

  /* Send messages to server; echo responses on stdout */
  for (int i = 2; i < argc; i++) {
    int msgLen = strlen(argv[i]);
    if (sendto(socket_fd, argv[i], msgLen, 0, (struct sockaddr *)&server_addr,
               sizeof(struct sockaddr_in)) != msgLen) {
      fprintf(stderr, "sendto() failed\n");
      exit(1);
    }

    char res_buf[BUF_SIZE];
    ssize_t numBytes = recvfrom(socket_fd, res_buf, BUF_SIZE, 0, NULL, NULL);
    if (numBytes == -1) {
      fprintf(stderr, "recvfrom() failed\n");
      exit(1);
    }

    printf("Response %d: %.*s\n", i - 1, (int)numBytes, res_buf);
  }

  return 0;
}

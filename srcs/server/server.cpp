#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT_NUM 50002

int main(int argc, char const *argv[]) {
  // 新規ソケットの作成
  int socket_fd;
  if (!(socket_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    fprintf(stderr, "socket() failed!\n");
    exit(1);
  }

  // well-knownアドレスにバインド
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_port = PORT_NUM;
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_fd, (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr_in)) == -1) {
    fprintf(stderr, "bind() failed!\n");
    exit(1);
  }

  // 接続要求を受け付けることをカーネルへ通知
  listen(socket_fd, SOMAXCONN);

  // クライアントからの接続要求を受け付け、コネクションを確立する。
  while (true) {
    int new_socket = accept(socket_fd, );
  }

  return 0;
}

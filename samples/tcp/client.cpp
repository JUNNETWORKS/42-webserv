#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char const *argv[]) {
  // 新規ソケットの作成
  int socket_fd;
  if (!(socket_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    fprintf(stderr, "socket() failed!\n");
    exit(1);
  }

  // コネクションを確立させる
  connect(socket_fd);

  return 0;
}

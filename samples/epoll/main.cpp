/* 多重I/Oを実現する epoll() システムコールのサンプル.
epoll() は監視対象のfdをカーネル空間で保持し続けるので select() や poll()
  よりもパフォーマンス的に優れている｡
epoll が監視できるfd(interest list)の数はユーザーごとに決まっており､
  /proc/sys/fs/epoll/max_user_watches で設定されている｡
epoll()はLinux独自のAPIなのでMacなどの他のUNIX環境では使えない可能性がある｡
使用例:
./a.out - 0r 1r
*/
#define _DEFAULT_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "utils.hpp"

static void usageError(const char *progName) {
  fprintf(stderr, "Usage: %s {timeout|-} fd-num[rw]...\n", progName);
  fprintf(stderr, "    - means infinite timeout; \n");
  fprintf(stderr, "    r = monitor for read\n");
  fprintf(stderr, "    w = monitor for write\n\n");
  fprintf(stderr, "    e.g.: %s - 0rw 1w\n", progName);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int ready, nfds, fd, numRead, i, j;
  int timeout;
  char buf[10]; /* Large enough to hold "rw\0" */

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageError(argv[0]);

  /* Timeout for epoll() is specified in argv[1] */

  if (strcmp(argv[1], "-") == 0) {
    timeout = -1; /* Infinite timeout */
  } else {
    timeout = atol(argv[1]);
  }

  // 引数はサイズを指定するものだが､この値を超えたディスクリプタも格納できる｡1以上なら何でも良い｡
  int epoll_fd = epoll_create(argc - 1);

  /* Process remaining arguments to build file descriptor sets */

  nfds = argc - 2;
  struct epoll_event *epevs =
      (struct epoll_event *)calloc(nfds, sizeof(struct epoll_event));
  for (i = 0, j = 2; j < argc; i++, j++) {
    numRead = sscanf(argv[j], "%d%2[rw]", &fd, buf);
    if (numRead != 2)
      usageError(argv[0]);
    if (fd >= FD_SETSIZE)
      errExit("file descriptor exceeds limit (%d)\n", FD_SETSIZE);
    epevs[i].data.fd = fd;
    if (strchr(buf, 'r') != NULL)
      epevs[i].events = EPOLLIN;
    if (strchr(buf, 'w') != NULL)
      epevs[i].events |= EPOLLOUT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epevs[i]);
  }

  /* We've built all of the arguments; now call epoll() */

  const int max_events = 100;
  struct epoll_event *received_events =
      (struct epoll_event *)calloc(max_events, sizeof(struct epoll_event));
  ready = epoll_wait(epoll_fd, received_events, max_events, timeout * 1000);
  /* Ignore exceptional events */
  if (ready == -1)
    errExit("select");

  /* Display results of epoll() */

  printf("ready = %d\n", ready);
  for (i = 0; i < ready; i++) {
    printf("%d: %s%s\n", received_events[i].data.fd,
           received_events[i].events & EPOLLIN ? "r" : "",
           received_events[i].events & EPOLLOUT ? "w" : "");
  }

  close(epoll_fd);  // 明示的にcloseする(学習用)
  exit(EXIT_SUCCESS);
}

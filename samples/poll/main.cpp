/* 多重I/Oを実現する poll() システムコールのサンプル
使用例: ./a.out - 0r 1r 9r 9< Makefile
*/
#define _DEFAULT_SOURCE
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  struct pollfd *fds;
  int ready, nfds, fd, numRead, i, j;
  int timeout;
  char buf[10]; /* Large enough to hold "rw\0" */

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageError(argv[0]);

  /* Timeout for poll() is specified in argv[1] */

  if (strcmp(argv[1], "-") == 0) {
    timeout = -1; /* Infinite timeout */
  } else {
    timeout = atol(argv[1]);
  }

  /* Process remaining arguments to build file descriptor sets */

  nfds = argc - 2;
  fds = (struct pollfd *)calloc(nfds, sizeof(struct pollfd));
  for (i = 0, j = 2; j < argc; i++, j++) {
    numRead = sscanf(argv[j], "%d%2[rw]", &fd, buf);
    if (numRead != 2)
      usageError(argv[0]);
    if (fd >= FD_SETSIZE)
      errExit("file descriptor exceeds limit (%d)\n", FD_SETSIZE);
    fds[i].fd = fd;
    if (strchr(buf, 'r') != NULL)
      fds[i].events |= POLLIN;
    if (strchr(buf, 'w') != NULL)
      fds[i].events |= POLLOUT;
  }

  /* We've built all of the arguments; now call poll() */

  ready = poll(fds, nfds, timeout * 1000);
  /* Ignore exceptional events */
  if (ready == -1)
    errExit("select");

  /* Display results of poll() */

  printf("ready = %d\n", ready);
  for (i = 0; i < nfds; i++) {
    printf("%d: %s%s\n", fds[i].fd, fds[i].revents & POLLIN ? "r" : "",
           fds[i].revents & POLLOUT ? "w" : "");
  }

  exit(EXIT_SUCCESS);
}

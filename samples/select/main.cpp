/* 多重I/Oを実現する select() システムコールのサンプル
使用例: ./a.out - 0r 1r 9r 9< Makefile
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
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
  fd_set readfds, writefds;
  int ready, nfds, fd, numRead, j;
  struct timeval timeout;
  struct timeval *pto;
  char buf[10]; /* Large enough to hold "rw\0" */

  if (argc < 2 || strcmp(argv[1], "--help") == 0)
    usageError(argv[0]);

  /* Timeout for select() is specified in argv[1] */

  if (strcmp(argv[1], "-") == 0) {
    pto = NULL; /* Infinite timeout */
  } else {
    pto = &timeout;
    timeout.tv_sec = atol(argv[1]);
    timeout.tv_usec = 0; /* No microseconds */
  }

  /* Process remaining arguments to build file descriptor sets */

  nfds = 0;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);

  for (j = 2; j < argc; j++) {
    numRead = sscanf(argv[j], "%d%2[rw]", &fd, buf);
    if (numRead != 2)
      usageError(argv[0]);
    if (fd >= FD_SETSIZE)
      errExit("file descriptor exceeds limit (%d)\n", FD_SETSIZE);

    if (fd >= nfds)
      nfds = fd + 1; /* Record maximum fd + 1 */
    if (strchr(buf, 'r') != NULL)
      FD_SET(fd, &readfds);
    if (strchr(buf, 'w') != NULL)
      FD_SET(fd, &writefds);
  }

  /* We've built all of the arguments; now call select() */

  ready = select(nfds, &readfds, &writefds, NULL, pto);
  /* Ignore exceptional events */
  if (ready == -1)
    errExit("select");

  /* Display results of select() */

  printf("ready = %d\n", ready);
  for (fd = 0; fd < nfds; fd++)
    printf("%d: %s%s\n", fd, FD_ISSET(fd, &readfds) ? "r" : "",
           FD_ISSET(fd, &writefds) ? "w" : "");

  if (pto != NULL)
    printf("timeout after select(): %ld.%03ld\n", (long)timeout.tv_sec,
           (long)timeout.tv_usec / 1000);
  exit(EXIT_SUCCESS);
}

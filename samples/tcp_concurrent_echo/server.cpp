#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "inet_sockets.hpp" /* Declarations of inet*() socket functions */
#include "utils.hpp"

#define SERVICE "echo" /* Name of TCP service */
#define BUF_SIZE 4096

/* SIGCHLD handler to reap dead child processses. ゾンビプロセス対策. */
static void grimReaper(int sig) {
  int savedErrno; /* save 'errno' in case changed here */

  savedErrno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0) continue;
  errno = savedErrno;
}

/* Handle a client request: copy socket input back to socket */
static void handleRequest(int cfd) {
  char buf[BUF_SIZE];
  ssize_t numRead;

  while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
    if (write(cfd, buf, numRead) != numRead)
      errExit("write() failed: %s", strerror(errno));
  }

  if (numRead == -1)
    errExit("Error from read(): %s", strerror(errno));
}

int main(int argc, char *argv[]) {
  int lfd, cfd; /* Listening and connected sockets */
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = grimReaper;
  if (sigaction(SIGCHLD, &sa, NULL) == -1)
    errExit("Error from sigaction(): %s", strerror(errno));

  lfd = inetListen(SERVICE, 10, NULL);
  if (lfd == -1)
    errExit("Could not create server socket (%s)", strerror(errno));

  while (true) {
    cfd = accept(lfd, NULL, NULL); /* Wait for connection */
    if (cfd == -1)
      errExit("Failure in accept(): %s", strerror(errno));

    /* Handle each client request in a new child process */
    switch (fork()) {
      case -1:
        fprintf(stderr, "Can't create child (%s)", strerror(errno));
        close(cfd);
        break;

      case 0:       /* child process */
        close(lfd); /* Unneeded copy of listening socket */
        handleRequest(cfd);
        exit(EXIT_SUCCESS);

      default:      /* parent process */
        close(cfd); /* Unneeded copy of connected socket */
        break;
    }
  }
}

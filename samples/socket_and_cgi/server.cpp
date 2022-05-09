#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "inet_sockets.hpp"
#include "utils.hpp"

#define PORT "8080"
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)

struct RequestContext {
  std::string method;
  std::string query_string;
};

// Set env params that is passed to the cgi program
// You can find what params is needed for cgi
//   at "The CGI Request" in RFC387(The Common Gateway Interface)
static void setCgiParams() {
  setenv("REQUEST_METHOD", "GET", 1);
  setenv("QUERY_STRING", "hoge&fuga=2%26", 1);
}

static void handleRequest(int cfd) {
  char *emptylist[] = {NULL};
  setCgiParams();

  // start CGI
  dup2(cfd, STDOUT_FILENO);
  dprintf(STDOUT_FILENO, "HTTP/1.0 200 OK\r\n");
  fflush(stdout);
  execve("./socket_and_cgi/flask.cgi", emptylist, environ);
}

static void printConnectionInfo(struct sockaddr_storage *addr,
                                socklen_t addrlen) {
  char addrStr[ADDRSTRLEN];
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  if (getnameinfo((struct sockaddr *)addr, addrlen, host, NI_MAXHOST, service,
                  NI_MAXSERV, 0) == 0) {
    snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
  } else {
    snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN?)");
  }
  printf("Connection from %s\n", addrStr);
}

int main(int argc, char const *argv[]) {
  int listen_fd = inetListen(PORT, 10, NULL);
  if (listen_fd == -1) {
    fatal("inetListen");
  }
  printf("%s listen at port %s\n", argv[0], PORT);

  while (1) {
    struct sockaddr_storage connection_addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    int connection_fd =
        accept(listen_fd, (struct sockaddr *)&connection_addr, &addrlen);
    if (connection_fd == -1) {
      errExit("accept");
    }
    printConnectionInfo(&connection_addr, addrlen);

    /* Handle each client request in a new child process */
    switch (fork()) {
      case -1:
        fprintf(stderr, "Can't create child (%s)", strerror(errno));
        close(connection_fd);
        break;

      case 0: /* Child */
        close(listen_fd);
        handleRequest(connection_fd);
        exit(EXIT_SUCCESS);

      default: /* Parent */
        close(connection_fd);
        break;
    }
  }

  return 0;
}

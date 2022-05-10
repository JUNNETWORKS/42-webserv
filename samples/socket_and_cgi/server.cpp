/* Tiny Web Server. CSAPP 11.6 */
//
// port 8080 で動かすとして
// リクエスト例(dynamic): GET http://localhost:8080/cgi-bin/flask.cgi
// リクエスト例(static): GET http://localhost:8080/README.md
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string>

#include "inet_sockets.hpp"
#include "read_line.hpp"
#include "utils.hpp"

#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
#define BUF_SIZE 2000

static void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                        char *longmsg) {
  char buf[BUF_SIZE], body[BUF_SIZE];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=\"ffffff\">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  write(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  write(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  write(fd, buf, strlen(buf));
  write(fd, body, strlen(body));
}

/* Return true if uri ponits static contents, otherwise return false */
static bool parseUri(char *uri, char *filename, char *cgiargs) {
  if (!strstr(uri, "cgi-bin")) { /* Static content */
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html");
    }
    return true;
  } else { /* Dynamic content */
    char *ptr = index(uri, '?');
    if (ptr) {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    } else {
      strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return false;
  }
}

static void getFileType(char *filename, char *filetype) {
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}

static void serveStatic(int fd, char *filename, int filesize) {
  /* Send response headers to client */
  char filetype[BUF_SIZE], buf[BUF_SIZE];
  getFileType(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  write(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client */
  int srcfd = open(filename, O_RDONLY);
  void *srcp = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  close(srcfd);
  write(fd, srcp, filesize);
  munmap(srcp, filesize);
}

static void serveDynamic(int fd, char *filename, char *cgiargs) {
  char buf[BUF_SIZE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  // flaskなどだとHTTPレスポンスステータスなどもflask側がクライアントに返すので注意｡
  // どっちが良いのかはよくわかってない｡(RFCに書いてるかも?)
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  write(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  write(fd, buf, strlen(buf));

  if (fork() == 0) {  // Child
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);
    setenv("REQUEST_METHOD", "GET", 1);
    dup2(fd, STDOUT_FILENO);               // Redirect stdout to client
    execve(filename, emptylist, environ);  // Run CGI program
  }
  wait(NULL);
}

static void handleRequest(int cfd) {
  /* Read request line and headers */
  char buf[BUF_SIZE], method[BUF_SIZE], uri[BUF_SIZE], version[BUF_SIZE];

  readLine(cfd, buf, BUF_SIZE);
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcmp(method, "GET") != 0) {
    /* Tiny Web Server only accept GET Request */
    clienterror(cfd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
    return;
  }

  // Skip reading headers. (Actually this process is necessary)
  do {
    readLine(cfd, buf, BUF_SIZE);
  } while (strcmp(buf, "\r\n"));

  /* Parse URI from GET request */
  char filename[BUF_SIZE], cgiargs[BUF_SIZE];
  bool is_static = parseUri(uri, filename, cgiargs);
  struct stat sbuf;
  if (stat(filename, &sbuf) < 0) {
    clienterror(cfd, method, "404", "Not Found",
                "Tiny couldn't find this file");
    return;
  }
  if (is_static) { /* Serve static content */
    if (!(S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))) {
      clienterror(cfd, filename, "403", "Forbidden",
                  "Tiny couldn't read the file");
      return;
    }
    serveStatic(cfd, filename, sbuf.st_size);
  } else { /* Serve dynamic content */
    if (!(S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))) {
      clienterror(cfd, filename, "403", "Forbidden",
                  "Tiny couldn't run the CGI program");
      return;
    }
    serveDynamic(cfd, filename, cgiargs);
  }
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
  /* Check command line args */
  if (argc != 2) {
    usageErr("usage %s <port>\n", argv[0]);
  }
  int listen_fd = inetListen(argv[1], 10, NULL);
  while (1) {
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int conn_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientlen);
    printConnectionInfo(&clientaddr, clientlen);
    handleRequest(conn_fd);
    close(conn_fd);
  }
}

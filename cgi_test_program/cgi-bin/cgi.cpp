/* CSAPP p778
  2つの引数を合計して返すCGIプログラム
  GET http://localhost?12&13 すると 25 を返す */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAXLINE 2000

int main(int argc, char const *argv[]) {
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the response body */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "<p>Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.</P>\r\n", content);
  sprintf(content, "%s<h1>The answer is: %d + %d = %d</h1>\r\n", content, n1,
          n2, n1 + n2);
  sprintf(content, "%s<p>Thanks for visiting!</p>\r\n", content);

  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
  return 0;
}

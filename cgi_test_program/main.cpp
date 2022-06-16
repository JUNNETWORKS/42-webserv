#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

extern char **environ;

// CGIの実行に必要な環境変数をセットする
// 想定
// URL http://localhost:80?12&13
//
// GET / HTTP/1.1
// Host: localhost
void SetCgiMetaVariables() {
  setenv("CONTENT_LENGTH", "0", 1);
  setenv("CONTENT_TYPE", "text", 1);
  setenv("GATEWAY_INTERFACE", "webserv", 1);
  setenv("PATH_INFO", "/", 1);
  setenv("PATH_TRANSLATED", "", 1);
  setenv("QUERY_STRING", "12&13", 1);
  setenv("REMOTE_ADDR", "127.0.0.1", 1);
  setenv("REMOTE_HOST", "127.0.0.1", 1);
  setenv("REQUEST_METHOD", "GET", 1);
  setenv("SCRIPT_NAME", "/cgi", 1);
  setenv("SERVER_NAME", "localhost", 1);
  setenv("SERVER_PORT", "80", 1);
  setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
  setenv("SERVER_SOFTWARE", "webserv/0.9", 1);
}

/* Exec cgi */
void ExecCgi(const char *cgi_path, int sockfd) {
  SetCgiMetaVariables();

  const char **argv = new const char *[2];
  argv[0] = cgi_path;
  argv[1] = NULL;
  execve(cgi_path, (char *const *)argv, (char *const *)environ);
  fprintf(stderr, "execve err\n");
  exit(1);
}

// fork して CGI スクリプトを実行する｡
// CGIスクリプトの標準出力がUnixDomainSocketに繋がるようにする｡
// 返り値はUnixDomainSocketのfd
//
// server <-- UnixDomainSocket --> cgi
int ForkAndExecCgi(const char *cgi_path) {
  int sockfd[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0) {
    exit(EXIT_FAILURE);
  }
  int parentsock = sockfd[0];
  int childsock = sockfd[1];

  pid_t pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    ExecCgi(cgi_path, childsock);
  }
  return parentsock;
}

int main(int argc, char const *argv[]) {
  // fork して CGI スクリプトを実行
  int sockfd = ForkAndExecCgi("./cgi-bin/cgi");

  // CGI スクリプトの出力を読み取る
  int read_num;
  do {
  } while (read_num > 0);

  // CgiResponseクラスを構築

  // できたCgiResponseオブジェクトを表示(デバッグ用)

  return 0;
}

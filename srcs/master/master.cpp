#include "master.hpp"

#include "configuration/configuration.hpp"
#include "utils/inet_sockets.hpp"
#include "worker/worker.hpp"

#define WORKER_NUM 1

namespace {

pid_t createWorkerProcess(int listen_fd) {
  pid_t pid = fork();
  switch (pid) {
    case -1:  // Error
      break;

    case 0:  // Child
      worker::startWorker(listen_fd);
      exit(0);
      break;

    default:  // Parent
      break;
  }
  return pid;
}

}  // namespace

int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;
  // TODO: 設定ファイルの読み込み. (設定ファイルの仕様がまとまってから書く)
  // struct Configuration config;

  // TODO: シグナルハンドラの設定

  // listen_fd を作成
  int listen_fd = utils::inetListen("8080", 10, NULL);
  std::cout << "Listen at 127.0.0.1:8080" << std::endl;

  // workerプロセスの生成
  for (int i = 0; i < WORKER_NUM; ++i) {
    pid_t pid = createWorkerProcess(listen_fd);
    if (pid == -1) {
      // もしworkerの生成に失敗したら全てのworkerを終了する
    }
  }

  // すべてのworkerプロセスが終了するまで待機する｡
  for (int i = 0; i < WORKER_NUM; ++i) {
    wait(NULL);
  }

  return 0;
}

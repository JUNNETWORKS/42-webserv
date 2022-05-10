# ソケットプログラミング学習用サンプル集

`make <dir_name>` で `server` と `client` という実行ファイルが作成されます(`utils` は対象外)｡

## サンプルの説明

- tcp: TCP通信を行うサーバー・クライアントプログラム
- tcp_concurrent_echo: 受け取ったリクエストごとにforkして処理するechoサーバー
- udp: UDP通信を行うサーバー・クライアントプログラム
- udp_iterative_echo: 受け取ったリクエストを親プロセスでそのまま処理するechoサーバー
- signal_driven_io: シグナルドリブンI/O の例
- select: `select()` の使用例
- poll: `poll()` の使用例
- epoll: `epoll()` の使用例
- epoll_with_socket: `epoll()` とソケット通信を組み合わせた例
- master_worker_epoll: Nginxのように master, worker プロセスを分けたサーバープログラム
- socket_and_cgi: CSAPP11.6 Tiny Web Server. リクエストURIを解析し､静的ファイル､CGI両方に対応している反復型のサーバープログラム


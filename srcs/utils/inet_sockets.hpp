#ifndef UTILS_INET_SOCKETS_HPP_
#define UTILS_INET_SOCKETS_HPP_

#include <netdb.h>
#include <sys/socket.h>

#include <string>

#include "result/result.hpp"
#include "server/socket_address.hpp"

// インターネットドメインソケットライブラリ

namespace utils {

using namespace result;

/* typeに指定されたソケットを作成し、
 * host, service に指定されたアドレスへ接続します。
 *
 * Args:
 *   host: ホスト名もしくは可読形式のIPアドレス(IPv4 or IPv6)。
 *         NULLを指定するとループバックアドレスを示すことになる。
 *   service: サービス名もしくは10進数の文字列で表現したポート番号を表す｡
 *   type: ソケット種類。 SOCK_STREAM または SOCK_DGRAM を指定する。
 *
 * Return:
 *   ファイルディスクリプタ。 エラーの場合は-1を返す。
 */
int InetConnect(const std::string &host, const std::string &service, int type);

/* ストリームソケットを作成し､serviceに指定されたTCPポートのワイルドカードアドレスへバインドし,リスニングソケットとする｡
 * TCPサーバ用の関数｡
 *
 * Args:
 *   service: サービス名もしくは10進数の文字列で表現したポート番号を表す｡
 *   backlog: 許容する保留コネクション数(listen()の引数と一緒)｡
 *   addrlen: 作成したソケットに対応する
 *            ソケットアドレス構造体のサイズを表す変数へのポインタ｡
 *
 * Return:
 *   ファイルディスクリプタ。 エラーの場合は-1を返す。
 */
Result<int> InetListen(const std::string &host, const std::string &service,
                       int backlog, server::SocketAddress *sockaddr);

/* typeに指定されたソケットを作成し、service､typeに指定されたポートのワイルドカードアドレスへバインドする｡
 * この関数はソケットを特定のアドレスへバインドするUDPサーバ､UDPクライアント用です｡
 *
 * Args:
 *   service: サービス名もしくは10進数の文字列で表現したポート番号を表す.
 *   type: ソケット種類。 SOCK_STREAM または SOCK_DGRAM を指定する。
 *   addrlen: 作成したソケットに対応する
 *            ソケットアドレス構造体のサイズを表す変数へのポインタ｡
 *
 * Return:
 *   ファイルディスクリプタ。 エラーの場合は-1を返す。
 */
Result<int> InetBind(const std::string &host, const std::string &service,
                     int type, server::SocketAddress *sockaddr);

/* インターネットソケットアドレスを可読形式に変換します｡
 * "(hostname, port-number)"の形式の文字列を返す｡
 *
 * Args:
 *   addr: 可読形式に変換したいソケットアドレス
 *   addrlen: addr の構造体のデータサイズ

 *
 * Return:
 *   結果の文字列を std::string で返す｡
 */
std::string InetAddressStr(const struct sockaddr *addr, socklen_t addrlen);

/* client_addrを元に "Connection from (<address>, <port>)\n"
 * のメッセージを標準出力に出力する｡ */
void LogConnectionInfoToStdout(struct sockaddr_storage &client_addr);

}  // namespace utils

#endif

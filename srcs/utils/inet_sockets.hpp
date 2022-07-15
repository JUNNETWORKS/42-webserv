#ifndef UTILS_INET_SOCKETS_HPP_
#define UTILS_INET_SOCKETS_HPP_

#include <netdb.h>
#include <sys/socket.h>

#include <string>

// インターネットドメインソケットライブラリ

namespace utils {

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
int InetListen(const std::string &service, int backlog, socklen_t *addrlen);

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
int InetBind(const std::string &service, int type, socklen_t *addrlen);

/* インターネットソケットアドレスを可読形式に変換します｡
 * addrStrに"(hostname, port-number)"の形式の文字列を格納する｡
 * addrStrが指すバッファに対してaddrlenバイトの領域を割り当てるのはコール側の責任です｡
 * 文字列が addrlen - 1 より大きくなる場合は切り詰められます｡
 * addrStrが指すバッファがすべての場合の文字列を格納できるサイズを
 * マクロ定数 IS_ADDR_STR_LEN として定義してあります｡
 *
 * Args:
 *   addr: 可読形式に変換したいソケットアドレス
 *   addrlen: addr の構造体のデータサイズ
 *   addrStr: 文字列を格納するためのバッファへのポインタ｡
 *            コール側で確保する必要がある｡
 *   addrStrLen: addrStr が指すバッファのサイズ
 *
 * Return:
 *   addrStr をそのまま返す｡
 */
char *InetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                     char *addrStr, int addrStrLen);

/* client_addrを元に "Connection from (<address>, <port>)\n"
 * のメッセージを標準出力に出力する｡ */
void LogConnectionInfoToStdout(struct sockaddr_storage &client_addr);

std::string GetSockaddrPort(const struct sockaddr_storage &addr);

std::string GetSockaddrIp(const struct sockaddr_storage &addr);

std::string GetSockaddrName(const struct sockaddr_storage &addr);

/* InetAddressStr() に指定する文字列のサイズ.
  (NI_MAXHOST + NI_MAXSERV + 4) よりも大きくなくてはならない. */
const int IS_ADDR_STR_LEN = 4096;

}  // namespace utils

#endif

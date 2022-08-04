# レビュー用資料

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [事前準備](#%E4%BA%8B%E5%89%8D%E6%BA%96%E5%82%99)
  - [42VM](#42vm)
  - [環境セットアップ](#%E7%92%B0%E5%A2%83%E3%82%BB%E3%83%83%E3%83%88%E3%82%A2%E3%83%83%E3%83%97)
- [レビュー (コンフィグ)](#%E3%83%AC%E3%83%93%E3%83%A5%E3%83%BC-%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0)
  - [コンフィグ](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0)
    - [コンフィグファイル受け取れる?](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8C%E3%82%8B)
    - [デフォルトコンフィグファイル読み取ろうとする?](#%E3%83%87%E3%83%95%E3%82%A9%E3%83%AB%E3%83%88%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E8%AA%AD%E3%81%BF%E5%8F%96%E3%82%8D%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B)
- [レビュー](#%E3%83%AC%E3%83%93%E3%83%A5%E3%83%BC)
  - [複数ポート対応](#%E8%A4%87%E6%95%B0%E3%83%9D%E3%83%BC%E3%83%88%E5%AF%BE%E5%BF%9C)
  - [1つのポートに複数のバーチャルサーバー, ドメイン名(Hostヘッダー)による分類](#1%E3%81%A4%E3%81%AE%E3%83%9D%E3%83%BC%E3%83%88%E3%81%AB%E8%A4%87%E6%95%B0%E3%81%AE%E3%83%90%E3%83%BC%E3%83%81%E3%83%A3%E3%83%AB%E3%82%B5%E3%83%BC%E3%83%90%E3%83%BC-%E3%83%89%E3%83%A1%E3%82%A4%E3%83%B3%E5%90%8Dhost%E3%83%98%E3%83%83%E3%83%80%E3%83%BC%E3%81%AB%E3%82%88%E3%82%8B%E5%88%86%E9%A1%9E)
  - [複数のホスト](#%E8%A4%87%E6%95%B0%E3%81%AE%E3%83%9B%E3%82%B9%E3%83%88)
  - [デフォルトエラーページ](#%E3%83%87%E3%83%95%E3%82%A9%E3%83%AB%E3%83%88%E3%82%A8%E3%83%A9%E3%83%BC%E3%83%9A%E3%83%BC%E3%82%B8)
  - [設定したエラーページ](#%E8%A8%AD%E5%AE%9A%E3%81%97%E3%81%9F%E3%82%A8%E3%83%A9%E3%83%BC%E3%83%9A%E3%83%BC%E3%82%B8)
  - [CGIが動くか](#cgi%E3%81%8C%E5%8B%95%E3%81%8F%E3%81%8B)
    - [Document-Response](#document-response)
    - [Local-Redirect](#local-redirect)
    - [Client-Redirect](#client-redirect)
  - [`.py` などの拡張子によってCGIを実行する](#py-%E3%81%AA%E3%81%A9%E3%81%AE%E6%8B%A1%E5%BC%B5%E5%AD%90%E3%81%AB%E3%82%88%E3%81%A3%E3%81%A6cgi%E3%82%92%E5%AE%9F%E8%A1%8C%E3%81%99%E3%82%8B)
  - [静的ファイルのサーブ](#%E9%9D%99%E7%9A%84%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%AE%E3%82%B5%E3%83%BC%E3%83%96)
  - [ファイルアップロード](#%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%82%A2%E3%83%83%E3%83%97%E3%83%AD%E3%83%BC%E3%83%89)
  - [client_body_size の制限](#client_body_size-%E3%81%AE%E5%88%B6%E9%99%90)
  - [許可したHTTPメソッドのみリソースへのアクセスが可能](#%E8%A8%B1%E5%8F%AF%E3%81%97%E3%81%9Fhttp%E3%83%A1%E3%82%BD%E3%83%83%E3%83%89%E3%81%AE%E3%81%BF%E3%83%AA%E3%82%BD%E3%83%BC%E3%82%B9%E3%81%B8%E3%81%AE%E3%82%A2%E3%82%AF%E3%82%BB%E3%82%B9%E3%81%8C%E5%8F%AF%E8%83%BD)
  - [root で指定したディレクトリからファイルを探す](#root-%E3%81%A7%E6%8C%87%E5%AE%9A%E3%81%97%E3%81%9F%E3%83%87%E3%82%A3%E3%83%AC%E3%82%AF%E3%83%88%E3%83%AA%E3%81%8B%E3%82%89%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%82%92%E6%8E%A2%E3%81%99)
  - [autoindex の on / off](#autoindex-%E3%81%AE-on--off)
  - [index ファイル](#index-%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB)
  - [`return` リダイレクト](#return-%E3%83%AA%E3%83%80%E3%82%A4%E3%83%AC%E3%82%AF%E3%83%88)
  - [複数言語でCGIスクリプトを動かす (ボーナス)](#%E8%A4%87%E6%95%B0%E8%A8%80%E8%AA%9E%E3%81%A7cgi%E3%82%B9%E3%82%AF%E3%83%AA%E3%83%97%E3%83%88%E3%82%92%E5%8B%95%E3%81%8B%E3%81%99-%E3%83%9C%E3%83%BC%E3%83%8A%E3%82%B9)
- [ベンチマーク](#%E3%83%99%E3%83%B3%E3%83%81%E3%83%9E%E3%83%BC%E3%82%AF)
  - [結果の読み方](#%E7%B5%90%E6%9E%9C%E3%81%AE%E8%AA%AD%E3%81%BF%E6%96%B9)
- [コンフィグファイルの説明](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%AE%E8%AA%AC%E6%98%8E)
  - [server1](#server1)
  - [server2](#server2)
  - [server3](#server3)
  - [server4](#server4)
  - [server5](#server5)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## 事前準備

### 42VM

42VM上でレビューを行うので42VMを起動できる状態にしておいてください｡ [42VM セットアップ](https://meta.intra.42.fr/articles/linux-vm)

### 環境セットアップ

実際に webserv を動かすための設定を行うシェルスクリプトを実行します｡

```
cd review && ./setup_review_env.sh
```

## レビュー (コンフィグ)

### コンフィグ

#### コンフィグファイル受け取れる?

`./webserv configurations/default.conf`

#### デフォルトコンフィグファイル読み取ろうとする?

`./webserv`

## レビュー

以下の項目をテストするコンフィグファイルを用意しています｡

`sudo ./webserv review/default.conf` と実行し､レビューを行ってください｡

- [ ] 複数ポート対応
- [ ] 1つのポートに複数のバーチャルサーバー
- [ ] ドメイン名(Hostヘッダー)による分類
- [ ] 複数のホスト
- [ ] デフォルトエラーページ
- [ ] 設定したエラーページ
- [ ] CGIが動くか
  - [ ] Document-Response
  - [ ] Local-Redirect
  - [ ] Client-Redirect
- [ ] `.py` などの拡張子によってCGIを実行する
- [ ] 静的ファイルのサーブ
- [ ] ファイルアップロード
- [ ] client_body_size の制限
- [ ] 許可したHTTPメソッドのみリソースへのアクセスが可能
- [ ] root で指定したディレクトリからファイルを探す
- [ ] autoindex の on / off
- [ ] index ファイル
- [ ] `return` リダイレクト
- [ ] 複数言語でCGIスクリプトを動かす (ボーナス)

### 複数ポート対応

`curl -v -X GET http://127.0.0.1:8080`

`curl -v -X GET http://127.0.0.1:9090`

### 1つのポートに複数のバーチャルサーバー, ドメイン名(Hostヘッダー)による分類

`curl -v -X GET http://localhost/`

`curl -v -X GET http://webserv.com/`

### 複数のホスト

`curl -v -X GET http://127.0.0.1:80/index.html`

`curl -v -X GET http://127.0.0.2:80/index.html`

### デフォルトエラーページ

`curl -v -X GET http://webserv.com/not_exists`

### 設定したエラーページ

`curl -v -X GET http://localhost/not_exists`

### CGIが動くか

[RFC 3875 - The Common Gateway Interface (CGI) Version 1.1 日本語訳](https://tex2e.github.io/rfc-translater/html/rfc3875.html)

#### Document-Response

`curl -v -X GET http://webserv.com/cgi-bin/document-response`

#### Local-Redirect

`curl -v -X GET http://webserv.com/cgi-bin/local-redirect`

#### Client-Redirect

`curl -v -X GET http://webserv.com/cgi-bin/client-redirect`

### `.py` などの拡張子によってCGIを実行する

`curl -v -X GET http://webserv.com/simple.py`

### 静的ファイルのサーブ

`curl -v -X GET http://localhost/index.html`

### ファイルアップロード

`curl -v -X POST --data-raw '@README.md' http://localhost/upload/README.md`

### client_body_size の制限

`curl -v -X POST --data-raw "$(python -c 'print("a" * 20000)')" http://localhost/upload`

### 許可したHTTPメソッドのみリソースへのアクセスが可能

`curl -v -X POST --data-raw '@README.md' http://localhost/`

### root で指定したディレクトリからファイルを探す

`curl -v -X GET http://localhost/index.html`

### autoindex の on / off

`curl -v -X GET http://127.0.0.2:80/`

### index ファイル

`curl -v -X GET http://localhost/`

### `return` リダイレクト

`curl -v -X GET http://localhost:9090/`
`curl -v -X GET http://localhost:9090/google`

### 複数言語でCGIスクリプトを動かす (ボーナス)

`curl http://webserv.com/cgi-bash/hello`

## ベンチマーク

`sudo ./webserv review/default.conf` でwebservを実行する｡

`siege -b http://localhost/index.html --time=30S` でベンチマークを実行する｡

オプションの意味
- `-b`: ベンチマークモード
- `--time=30S` 30秒間ベンチマークを実行する｡

ちなみにデフォルトではsiegeは25クライアント並行で動かす

### 結果の読み方

```
Transactions:                  87319 hits
Availability:                 100.00 %
Elapsed time:                  29.00 secs
Data transferred:               9.58 MB
Response time:                  0.01 secs
Transaction rate:            3011.00 trans/sec
Throughput:                     0.33 MB/sec
Concurrency:                   24.93
Successful transactions:       87319
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```

- Transactions: ベンチマークで送信されたリクエスト数
- Availability: レスポンスが返ってきて､なおかつレスポンスコード400や500ではないレスポンスの送信したリクエスト数に対する割合
- Elapsed time: ベンチマークを実行した時間
- Data transferred: ヘッダーを含むsiegeの各クライアントに送信されたデータ量の総和
- Response time: レスポンスが返ってくるまでの平均時間
- Transaction rate: サーバーが1秒間に処理出来る平均リクエスト数
- Throughput: サーバーがsiegeの各クライアントに1秒間に送信する平均データ量
- Concurrency: 同時に接続されたコネクション数の平均｡ サーバーの性能が悪いほどこの数値はあがる｡
- Successful transactions: ステータスコードが400より下(つまり正常)のレスポンス数
- Failed transactions: 400以上のステータスまたはソケットに関してエラーが発生した数
- Longest transaction: レスポンスを返すのに要した時間の最大
- Shortest transaction: レスポンスを返すのに要した時間の最小

## コンフィグファイルの説明

レビューで使う `review/default.conf` の説明

### server1

- エラーページの設定
- 127.0.0.1:80 に対するリクエストでserver_nameがどのバーチャルサーバーとも一致しなかった場合のデフォルトバーチャルサーバー(一番上に書いてあるので)
- index
- root
- ファイルアップロード
- リクエストボディサイズの制限
- 許可するHTTPメソッド
- autoindex

### server2

- CGI
- 後方一致によるlocation取得

### server3

- 127.0.0.2:80
- server1, server2とは違うIPでのlisten

### server4

- listen にポートのみ指定した場合｡この場合は自分の持つ全ネットワークインターフェースにポートをバインドする｡

### server5

- return ディレクトリによるリダイレクト

# レビュー用資料

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

  - [コンフィグ](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0)
    - [コンフィグファイル受け取れる?](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8C%E3%82%8B)
    - [デフォルトコンフィグファイル読み取ろうとする?](#%E3%83%87%E3%83%95%E3%82%A9%E3%83%AB%E3%83%88%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E8%AA%AD%E3%81%BF%E5%8F%96%E3%82%8D%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B)
- [レビュー確認事項](#%E3%83%AC%E3%83%93%E3%83%A5%E3%83%BC%E7%A2%BA%E8%AA%8D%E4%BA%8B%E9%A0%85)

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

`./webserv configurations/simple.conf`

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

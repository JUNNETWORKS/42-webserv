# Configuration の仕様

webservで用いる設定ファイルの仕様について述べる｡

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [基本](#%E5%9F%BA%E6%9C%AC)
- [server](#server)
  - [listen](#listen)
  - [server_name](#server_name)
  - [location](#location)
    - [allow_method](#allow_method)
    - [client_max_body_size](#client_max_body_size)
    - [root](#root)
    - [index](#index)
    - [is_cgi](#is_cgi)
    - [cgi_executor](#cgi_executor)
    - [error_page](#error_page)
    - [autoindex](#autoindex)
    - [return](#return)
- [サンプル](#%E3%82%B5%E3%83%B3%E3%83%97%E3%83%AB)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## 基本

ファイルの拡張子は `.conf` である｡

正規表現は正規表現エンジンは作る予定無いので対応してない｡

変数についても対応しない｡

## server

- Required: True
- Multiple: True

Syntax: `server {}`

serverブロック｡これ1つでバーチャルサーバ1つを表す｡

以下の項目が設定可能. (末尾に`{}`がついているものはブロック)

- listen
- server_name
- location{}

また､serverブロックは複数書くことで複数のバーチャルサーバを立てることができる｡

### listen

- Required: True
- Multiple: False

Syntax: `listen <port_number>;` or `listen <address>:<port_number>`

その対象のバーチャルサーバでlistenする｡


### server_name

- Required: False
- Multiple: False

Syntax: `server_name: <host_name> [<host_name>...]`

バーチャルサーバで使用するホスト名を指定する｡

これを設定すると"HTTP Hostヘッダー" を元にどのバーチャルサーバと通信を行うか振り分けられる｡

複数のバーチャルサーバの優先順位は以下のようになっている｡

1. listenディレクティブのアドレスとポートに一致するバーチャルサーバを検索する
1. リクエストのHostヘッダが`server_name`ディレクティブで指定したホスト一致したバーチャルサーバにリクエストを振り分ける
1. どのサーバにも一致しない場合デフォルトサーバにリクエストを振り分ける｡ デフォルトサーバは設定ファイルの一番上に記述したバーチャルサーバが使用される｡

### location

- Required: False
- Multiple: True

Syntax: `location <pattern> {}` or `location_back <pattern> {}`

`location` ディレクティブは前方一致でマッチングを行う｡ `<pattern>` には正規表現は使えず､純粋に前方一致のみ行われることに注意｡

`location_back` ディレクティブは後方一致でマッチングを行う｡ 拡張子でルーティングしたい場合などに使う想定｡ ブロック内で利用できるディレクティブなどは`location`と同じ｡

locationの振り分けの優先順位
1. 前方一致のlocationを判定し､最も長い文字列の`<pattern>`にマッチしたものを選ぶ｡

#### allow_method

- Required: False
- Multiple: False

Syntax: `allow_method <method> [<method>...];`

指定した `<method>` のみを許可する｡ 指定しない場合はすべてのメソッドを拒絶する｡

ディレクティブ引数の `<method>` は1つ以上必要｡

`<method>` は以下の文字列のみ許可する
- `GET`
- `POST`
- `DELETE`

#### client_max_body_size

- Required: False
- Multiple: False

Syntax: `client_max_body_size <size>;`

受信できるリクエストボディの最大サイズをバイト単位で指定する｡

#### root

- Required: True
- Multiple: False

Syntax: `root <path>;`

公開するディレクトリを指定する｡

#### index

- Required: False
- Multiple: False

Syntax: `index <path> [<path>...];`

リクエスト対象がディレクトリだった場合にレスポンスとして返すファイル
`is_cgi on;` の場合は無視される｡

#### is_cgi

- Required: False
- Multiple: False

Syntax: `is_cgi <on_or_off>;`

`is_cgi on;` にするとその対象の`location`ディレクティブに入ってくるリクエストはCGIへのリクエストと解釈される｡

指定しない場合は `is_cgi off;` と同じ扱い｡

#### cgi_executor

- Required: True (if `is_cgi on;`)
- Multiple: False

syntax: `cgi_executor <path_or_file_name>`

`<path_or_file_name>` は `execvpe()` の第一引数に渡される文字列｡

#### error_page

- Required: False
- Multiple: True

Syntax: `error_page <status_code> [<status_code>...] <error_page_path>`

`<status_code>` 時に `<error_page_path>` で指定されたファイルを返す｡

#### autoindex

- Required: False
- Multiple: False

Syntax: `autoindex <on_or_off>;`

`autoindex on;` の場合､リクエスト先がディレクトリだった場合にディレクトリ内ファイル一覧ページを返す｡

指定しない場合は `autoindex off;` と同じ扱い｡

#### return

- Required: False
- Multiple: False

Syntax: `return <path>;`

HTTPステータスコード301と共に`<path>`へリダイレクトする｡

e.g. `return http://localhost/index.html;`

## サンプル

```
server {
  listen 80;
  server_name localhost;

  location / {
    allow_method GET;

    root /var/www/html;
    index index.html index.htm;

    error_page 500 /server_error_page.html;
    error_page 404 403 /not_found.html;
  }

  location /upload {
    allow_method GET POST DELETE;

    client_max_body_dize 1M;

    root /var/www/user_uploads;
    autoindex on;
  }
}

server {
  listen 80;
  server_name www.webserv.com webserv.com;

  location / {
    root /var/www/html;
    index index.html;
  }

  location_back .php {
    is_cgi on;
    cgi_executor php-cgi;
    root /home/nginx/cgi_bins;
  }
}

server {
  listen 8080;
  server_name localhost;

  location / {
    root /var/www/html;
    index index.html;
  }
}

server {
  listen 9090;

  location / {
    return http://localhost:8080/;
  }
}
```

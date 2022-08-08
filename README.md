# 42-webserv

Webserv is one of the projects in 42 cursus.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Docs](#docs)
- [使い方](#%E4%BD%BF%E3%81%84%E6%96%B9)
- [コーディングルール](#%E3%82%B3%E3%83%BC%E3%83%87%E3%82%A3%E3%83%B3%E3%82%B0%E3%83%AB%E3%83%BC%E3%83%AB)
- [キーワード](#%E3%82%AD%E3%83%BC%E3%83%AF%E3%83%BC%E3%83%89)
- [サーバーの流れ](#%E3%82%B5%E3%83%BC%E3%83%90%E3%83%BC%E3%81%AE%E6%B5%81%E3%82%8C)
  - [HTTP STATE MACHINE](#http-state-machine)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Docs

- [configuration.md](docs/configuration.md): コンフィグファイルの仕様
- [review.md](docs/review.md): レビュー用資料

## 使い方

`make re && ./webserv [config path]` で起動｡

何ができるかは[review.md](docs/review.md)を参照してください｡

## コーディングルール

[Google C++ Style Guide](https://ttsuki.github.io/styleguide/cppguide.ja.html) に従う｡

## ディレクトリ構成

```
.
 configurations: コンフィグファイル置き場
 docs: ドキュメント
 review: レビュー用のファイルや設定
 srcs
    cgi: CGIに関するコード｡CGI実行やCGIの出力のパースなど｡
    config: コンフィグクラスの定義やコンフィグパーサー｡
    http: HTTPのリクエストパーサーや各種HttpResponseクラス｡
    result: Result<T> の宣言と実装
    server: main関数や初期設定､イベントループ､ソケットクラスの定義など｡
    utils: 便利関数｡
 test: 他のサーバーソフトウェアとの出力比較などの動作確認用｡
 unit_test: ユニットテスト｡
```

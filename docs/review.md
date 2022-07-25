# レビュー用資料

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

  - [コンフィグ](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0)
    - [コンフィグファイル受け取れる?](#%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8C%E3%82%8B)
    - [デフォルトコンフィグファイル読み取ろうとする?](#%E3%83%87%E3%83%95%E3%82%A9%E3%83%AB%E3%83%88%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E8%AA%AD%E3%81%BF%E5%8F%96%E3%82%8D%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B)
- [レビュー確認事項](#%E3%83%AC%E3%83%93%E3%83%A5%E3%83%BC%E7%A2%BA%E8%AA%8D%E4%BA%8B%E9%A0%85)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

### コンフィグ

#### コンフィグファイル受け取れる?

`./webserv simple.conf`

#### デフォルトコンフィグファイル読み取ろうとする?

`./webserv default.conf`

## レビュー確認事項

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

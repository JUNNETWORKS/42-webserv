# レビュー用資料

## Docker の外でやる

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

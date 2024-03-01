# ChatRoom Server

C言語製チャットルームアプリ用自作サーバ

## 使い方

パスに対してハンドラを登録し，サーバを起動する．

```c
SERVER server = server_create();

server_add_path(&server, "/", root_handler);
server_add_path(&server, "/games", root_handler);
server_add_path(&server, "/*", room2_handler);

server_start(&server, 8080);
```

Hono (<https://github.com/honojs/hono>) というライブラリからインスパイアされた．

```ts
import { Hono, Middleware } from 'hono'

const app = new Hono()

app.use('*', Middleware.logger())

app.get('/', (c) => c.text('Hello Hono!'))
app.get('/entry/:id', (c) => {
  return c.json({ 'your id' : c.req.param('id') })
})

app.fire()
```

## ビルド

```sh
make
```

## 仕様

### ライブラリ

- `server_create()` でサーバのインスタンスを作成．
- `server_add_path()` でパスに対してハンドラを登録．
- パスが存在する場合はそのハンドラを呼び出す．
- 直接的なパスが存在しない場合は，ワイルドカード `/*` に対するハンドラを呼び出す．
- `/404` に対するハンドラが存在しない場合は，デフォルトのハンドラを呼び出す．
- `server_start()` でサーバを起動．

### HTTP

- `GET /` : チャットルーム一覧の JSON を返す
  
  ```json
  [ "room1", "room2", "room3", ... ]
  ```

- `POST /` : 新規チャットルームの作成

  ```json
  { "room": "doly lab" }
  ```

- `GET /{room}` : チャットルームのメッセージ一覧の JSON を返す

  ```json
  {
      "room": "room name",
      "messages": [
          {
              "id": "tom",
              "timestamp": "2020-12-24 12:00:00",
              "message": "Yo"
          },
          {
              "id": "jerry",
              "timestamp": "2020-12-24 12:00:00",
              "message": "Hi!"
          },

          ...
      ]
  }
  ```

- `POST /{room}` : チャットルームにメッセージを送信

  ```json
  {
      "id": "tom",
      "message": "Yo"
  }
  ```

## 使用したライブラリ

- sqlite3: データベース
- sarson: JSON パーサ

## 工夫した点

- 関数ポインタを利用した簡潔な見た目を実現．
- ワイルドカード `/*` による動的にルーティング．
- 構造体，関数，ディレクトリ構成，エイリアスを考えて設計．
- なるべく `if` や `for` を秘匿し，インデントが深くならないように関数型?(使ったことない)のようなコーディングを意識．
- 同じスコープ内でリソースの開放によるメモリリークの防止．
- 変数スコープを短くした．
- エラーハンドリングもそこそこ意識した．
- clang-format を使用．

## 課題

- Makefile の理解が浅くハンドラが追加しにくい．
  - ハンドラを追加したときに Makefile を書き換えないようにしたい．
- yaml などの設定ファイルを読み込んで，ハンドラを登録するようにしたい．
- C言語でテストするのは大変そうなのでぶっつけ本番で作ってしまった．
- ネストされたワイルドカードに対応していない．
  - 再帰的な構造体にして実現可能．
- clang-format で気に入らないフォーマットが残っている．
- HTTP の実装が最低限．
  - ヘッダを全く使っていない．

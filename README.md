# ChatRoom Server

C言語製チャットルームアプリ用自作サーバ

## 使い方

パスに対してハンドラを登録し，サーバを起動する．

```c
SERVER server = server_create();

server_add_path(&server, "/", root_handler);
server_add_path(&server, "/games", root_handler);
server_add_path(&server, "/*", room_handler);

server_start(&server, 8080);
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
- `server_start()` でサーバを起動．(リクエスト毎に fork)
- `server_start_thread()` でサーバを起動．(リクエスト毎にスレッドを生成)

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

- パスによって実行する関数を決めるようにした．
- 簡潔なライブラリの使い方．
- ワイルドカード `/*` による動的にルーティング．

## 課題

- Makefile の理解が浅くハンドラが追加しにくい．
  - ハンドラを追加したときに Makefile を書き換えないようにしたい．
- yaml などの設定ファイルを読み込んで，ハンドラを登録するようにしたい．
- C言語でテストするのは大変そうなのでぶっつけ本番で作ってしまった．
- パスをネストさせる記述を工夫したい．
  - 構造体を再帰的なものにする．
  - ルート以外のワイルドカードに対応させる．
- clang-format で気に入らないフォーマットが残っている．
- HTTP の実装が最低限．
  - ヘッダを全く使っていない．
- データベース系の処理をハンドラと同じファイルに書けばよかった．

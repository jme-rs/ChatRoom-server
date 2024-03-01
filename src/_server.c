#include "db.h"
#include "parson/parson.h"
#include "server.h"
#include "server_helper.h"


static void fork_server(int newfd, sqlite3 *db);

void server(int port)
{
    // サーバソケットを作成
    int sockfd = get_server_socket(port);
    if (sockfd < 0)
        return;

    // データベースと接続
    sqlite3 *db;
    int      rc = sqlite3_open("chatroom.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // HTTP サーバを生み出す
    printf("Server listening on port %d...\n\n", port);

    while (true) {
        int newfd = accept_connection(sockfd);
        if (newfd == -1)
            break;

        pid_t pid = fork();
        switch (pid) {
            case -1: {
                perror("fork");
                close(newfd);
                goto exit_process;
            }

            // child
            case 0: {
                fork_server(newfd, db);
                close(newfd);
                goto exit_process; // 子プロセスは終了
            }

            // parent
            default: {
                // waitpid(pid, NULL, 0); // todo
                close(newfd);
                break;
            }
        }
    }

exit_process: // 終了処理
    close(sockfd);
    sqlite3_close(db);
}


// fork された子プロセス
// 1 リクエストに対して 1 応答 1 プロセス
static void fork_server(int newfd, sqlite3 *db)
{
    char recv_buf[BUFSIZ];

    // 受信
    ssize_t receive_n = recv(newfd, recv_buf, BUFSIZ, 0);
    if (receive_n < 0) {
        perror("recv");
        return;
    }
    // printf("** Received from client **\n"
    //        "%s\n"
    //        "** end **\n\n",
    //        recv_buf);

    // HTTP メソッドの特定
    char *saveptr;                                    // strtok_r 用
    char *method = strtok_r(recv_buf, " ", &saveptr); // "GET" or "POST"

    // パスの特定
    char *path = strtok_r(NULL, " ", &saveptr);

    // body を取得
    char *body;
    while ((body = strtok_r(NULL, "\n", &saveptr)) != NULL) {
        if (strlen(body) == 1)
            break;
    }
    body = strtok_r(NULL, "\0", &saveptr);

    // GET メソッドの場合
    if (strcmp(method, "GET") == 0) {

        // root はチャットルーム一覧の取得
        if (strcmp(path, "/") == 0) {

            // ログ
            printf("Request: get room list\n");

            // データベースからルーム名を取得
            JSON_Value *rooms = db_select_rooms(db);

            // 送信用の JSON を作成
            //
            // send_str == [
            //     "room1", "room2", "room3", ...
            //]
            //
            char *send_str = json_serialize_to_string(rooms);

            // 送信するjson文字列の長さに応じてバッファを確保
            int   send_str_len = strlen(send_str);
            char *send_buf     = (char *) malloc(send_str_len + 128);

            sprintf(send_buf,
                    "HTTP/1.1 200\n"
                    "\n"
                    "%s\n",
                    send_str);

            // 送信
            ssize_t send_n = send(newfd, send_buf, strlen(send_buf), 0);
            if (send_n < 0) {
                perror("send");
                return;
            }

            // free
            json_free_serialized_string(send_str);
            json_value_free(rooms);
            free(send_buf);
        }

        // パスはルーム名
        else if (true) { // todo

            // ログ
            printf("Request: get %s messages\n", path);

            // ルーム名を取得 (先頭の '/' を削除)
            char *room_name = &path[1];

            // データベースからメッセージを取得
            JSON_Value *send_messages = db_select_msg(db, room_name);

            // 送信用の JSON を作成
            //
            // send_str == {
            //     "room": "room name",
            //     "messages": [
            //         {
            //             "id": "tom1224",
            //             "timestamp": "2020-12-24 12:00:00",
            //             "message": "Hello, World!"
            //         },
            //         ...
            //     ]
            // }
            //
            JSON_Value  *send_value  = json_value_init_object();
            JSON_Object *send_object = json_value_get_object(send_value);
            JSON_Value  *room        = json_value_init_string(room_name);
            json_object_set_value(send_object, "room", room);
            json_object_set_value(send_object, "message", send_messages);

            // 送信用の JSON を文字列に変換
            char *send_str = json_serialize_to_string(send_value);

            // 送信するjson文字列の長さに応じてバッファを確保
            int   send_str_len = strlen(send_str);
            char *send_buf     = (char *) malloc(send_str_len + 128);

            sprintf(send_buf,
                    "HTTP/1.1 200\n"
                    "\n"
                    "%s\n",
                    send_str);

            // デバッグ用
            // printf("** Send to client **\n"
            //        "%s\n"
            //        "** end **\n\n",
            //        send_buf);

            // 送信
            ssize_t send_n = send(newfd, send_buf, strlen(send_buf), 0);
            if (send_n < 0) {
                perror("send");
                return;
            }

            // free
            json_free_serialized_string(send_str);
            json_value_free(send_value);
            free(send_buf);
        }

        // 404
        else {
            char send_buf[BUFSIZ];
            sprintf(send_buf, "HTTP/1.1 404\n\n");

            // 送信
            ssize_t send_n = send(newfd, send_buf, strlen(send_buf), 0);
            if (send_n < 0) {
                perror("send");
                return;
            }
        }
    }

    // POST の場合
    else if (strcmp(method, "POST") == 0) {

        // root はルームの作成
        if (strcmp(path, "/") == 0) {
            char send_buf[BUFSIZ];

            // JSON を解析
            JSON_Value  *json_value  = json_parse_string(body);
            JSON_Object *json_object = json_value_get_object(json_value);
            const char  *room = json_object_get_string(json_object, "room");

            // ログ
            printf("Request: create room '%s'\n", room);

            // データベースにルームを挿入
            int rc = db_insert_rooms(db, room);
            if (rc != 0) {
                sprintf(send_buf, "HTTP/1.1 500\n\n");
            }
            else {
                sprintf(send_buf, "HTTP/1.1 200\n\n");
            }

            // 送信
            ssize_t send_n = send(newfd, send_buf, strlen(send_buf), 0);
            if (send_n < 0) {
                perror("send");
                return;
            }

            // free
            json_value_free(json_value);
        }

        // パスはルーム名
        else if (true) { // todo
            char send_buf[BUFSIZ];

            // ログ
            printf("Request: post message to %s\n", path);

            // ルーム名を取得 (先頭の '/' を削除)
            char *room_name = &path[1];

            // JSON を解析
            JSON_Value  *json_value  = json_parse_string(body);
            JSON_Object *json_object = json_value_get_object(json_value);
            const char  *id  = json_object_get_string(json_object, "id");
            const char  *msg = json_object_get_string(json_object, "message");

            // データベースにメッセージを挿入
            int rc = db_insert_msg(db, id, room_name, msg);
            if (rc != 0) {
                sprintf(send_buf, "HTTP/1.1 500\n\n");
            }
            else {
                sprintf(send_buf, "HTTP/1.1 200\n\n");
            }

            // 送信
            ssize_t send_n = send(newfd, send_buf, strlen(send_buf), 0);
            if (send_n < 0) {
                perror("send");
                return;
            }

            // free
            json_value_free(json_value);
        }
    }

    // プロセス終了
}

#include "rooms.h"


static int get(int fd, sqlite3 *db, const HTTP_REQUEST *request);
static int post(int fd, sqlite3 *db, const HTTP_REQUEST *request);


int rooms_handler(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    if (strcmp(request->method, "GET") == 0)
        return get(fd, db, request);
    else if (strcmp(request->method, "POST") == 0)
        return post(fd, db, request);
    else
        return -1;
}


static int get(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    // 先頭の / を取り除く
    const char *room_name = &request->path[1];

    // データベースからメッセージを取得
    JSON_Value *send_messages = db_select_msg(db, room_name);

    // 送信用 JSON を作成
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
    char *send_json_str = json_serialize_to_string(send_value);

    // HTTP レスポンスの作成
    HTTP_RESPONSE response = {
        .status = 200,
        .body = send_json_str
    };

    // 送信
    int ret = send_response(fd, &response);

    // 終了処理
    json_free_serialized_string(send_json_str);
    json_value_free(send_value);

    return ret;
}


static int post(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    // 先頭の / を取り除いてルーム名を取得
    const char *room_name = &request->path[1];

    // JSON を解析
    JSON_Value  *json_value  = json_parse_string(request->body);
    JSON_Object *json_object = json_value_get_object(json_value);
    const char  *id          = json_object_get_string(json_object, "id");
    const char  *msg         = json_object_get_string(json_object, "message");

    // データベースにメッセージを挿入
    int rc = db_insert_msg(db, id, room_name, msg);

    // HTTP レスポンスの作成
    HTTP_RESPONSE response = (rc < 0)
                               ? (HTTP_RESPONSE){ .status = 500, .body = "" }
                               : (HTTP_RESPONSE){ .status = 200, .body = "" };

    // 送信
    int ret = send_response(fd, &response);

    // 終了処理
    json_value_free(json_value);

    return ret;
}

#include "root.h"


static int get(int fd, sqlite3 *db);
static int post(int fd, sqlite3 *db, const HTTP_REQUEST *request);


int root_handler(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    if (strcmp(request->method, "GET") == 0)
        return get(fd, db);
    else if (strcmp(request->method, "POST") == 0)
        return post(fd, db, request);
    else
        return -1;
}


static int get(int fd, sqlite3 *db)
{
    // データベースからルーム名一覧を取得
    JSON_Value *rooms    = db_select_rooms(db);
    //
    // rooms_json_str == [
    //     "room1", "room2", "room3", ...
    //]
    //
    char *rooms_json_str = json_serialize_to_string(rooms);

    // HTTP レスポンスの作成
    HTTP_RESPONSE response = { .status = 200, .body = rooms_json_str };

    // 送信
    int ret = send_response(fd, &response);

    // 終了処理
    json_value_free(rooms);
    json_free_serialized_string(rooms_json_str);

    return ret;
}


static int post(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    // リクエストボディの JSON をパース
    JSON_Value  *body_json_value  = json_parse_string(request->body);
    JSON_Object *body_json_object = json_value_get_object(body_json_value);
    const char  *room = json_object_get_string(body_json_object, "room");

    // ルーム名をデータベースに挿入
    int rc = db_insert_rooms(db, room);

    // HTTP レスポンスの作成
    HTTP_RESPONSE response = (rc < 0)
                               ? (HTTP_RESPONSE){ .status = 500, .body = "" }
                               : (HTTP_RESPONSE){ .status = 200, .body = "" };

    // 送信
    int ret = send_response(fd, &response);

    // 終了処理
    json_value_free(body_json_value);

    return ret;
}

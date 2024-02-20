#include "db.h"
#include "server_helper.h"


// rooms(name text)
JSON_Value *db_select_rooms(sqlite3 *db)
{
    // ステートメントの作成
    sqlite3_stmt *pStmt;

    int rc = sqlite3_prepare_v2(db, "SELECT name FROM rooms", -1, &pStmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // json array の作成
    JSON_Value *json_value = json_value_init_array();
    JSON_Array *json_array = json_value_get_array(json_value);

    // JSON の作成
    while (SQLITE_ROW == sqlite3_step(pStmt)) {
        const char *name = (const char *) sqlite3_column_text(pStmt, 0);

        JSON_Value *json_value = json_value_init_string(name);
        json_array_append_value(json_array, json_value);
    }
    sqlite3_finalize(pStmt);

    return json_value;
}


// rooms(name text)
int db_insert_rooms(sqlite3 *db, const char *room)
{
    // ステートメントの作成
    sqlite3_stmt *pStmt;

    int rc = sqlite3_prepare_v2(db,
                                "INSERT INTO rooms (name) VALUES (?)",
                                -1,
                                &pStmt,
                                NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // 値の挿入
    sqlite3_bind_text(pStmt, 1, room, strlen(room), SQLITE_TRANSIENT);
    while (SQLITE_DONE != sqlite3_step(pStmt)) {}
    sqlite3_finalize(pStmt);

    return 0;
}


// messages(
//     id text,
//     room text,
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
//     message text
// );
int db_insert_msg(sqlite3    *db,
                  const char *id,
                  const char *room,
                  const char *msg)
{
    // ステートメントの作成
    sqlite3_stmt *pStmt;

    int rc = sqlite3_prepare_v2(
        db,
        "INSERT INTO messages (id, room, message) VALUES (?, ?, ?)",
        -1,
        &pStmt,
        NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // 値の挿入
    sqlite3_bind_text(pStmt, 1, id, strlen(id), SQLITE_TRANSIENT);
    sqlite3_bind_text(pStmt, 2, room, strlen(room), SQLITE_TRANSIENT);
    sqlite3_bind_text(pStmt, 3, msg, strlen(msg), SQLITE_TRANSIENT);
    while (SQLITE_DONE != sqlite3_step(pStmt)) {}
    sqlite3_finalize(pStmt);

    return 0;
}


// messages(
//     id text,
//     room text,
//     timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
//     message text
// );
JSON_Value *db_select_msg(sqlite3 *db, const char *room)
{
    // ステートメントの作成
    sqlite3_stmt *pStmt;

    int rc = sqlite3_prepare_v2(
        db,
        "SELECT id, timestamp, message FROM messages WHERE room = ?",
        -1,
        &pStmt,
        NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // json array の作成
    JSON_Value *json_value = json_value_init_array();
    JSON_Array *json_array = json_value_get_array(json_value);

    // JSON の作成
    sqlite3_bind_text(pStmt, 1, room, strlen(room), SQLITE_TRANSIENT);
    while (SQLITE_ROW == sqlite3_step(pStmt)) {
        const char *id        = (const char *) sqlite3_column_text(pStmt, 0);
        const char *timestamp = (const char *) sqlite3_column_text(pStmt, 1);
        const char *message   = (const char *) sqlite3_column_text(pStmt, 2);

        // printf("id: %s, timestamp: %s, message: %s\n", id, timestamp,
        // message);

        JSON_Value  *json_value  = json_value_init_object();
        JSON_Object *json_object = json_value_get_object(json_value);
        json_object_set_string(json_object, "id", id);
        json_object_set_string(json_object, "timestamp", timestamp);
        json_object_set_string(json_object, "message", message);
        json_array_append_value(json_array, json_value);
    }
    sqlite3_finalize(pStmt);

    return json_value;
}

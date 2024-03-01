#include "not_found.h"


int not_found_handler(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    // HTTP レスポンスの作成
    HTTP_RESPONSE response = { .status = 404, .body = "Not Found" };

    // 送信
    int ret = send_response(fd, &response);

    return ret;
}

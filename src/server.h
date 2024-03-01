#ifndef SERVER_H
#define SERVER_H


#include <netinet/in.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server_helper.h"

#define MAX_PATH 16


// ハンドラの関数型エイリアス
typedef int (*handler_f)(int, sqlite3 *, const HTTP_REQUEST *);

// パスとハンドラの対応
typedef struct {
    char     *path_name;
    handler_f handler;
} SERVER_PATH;

// サーバ
// パスとハンドラのリスト
typedef struct {
    SERVER_PATH paths[MAX_PATH];
    int         path_count;
} SERVER;

SERVER server_create();
void   server_set_path(SERVER *server, char *path_name, handler_f handler);
void   server_start(SERVER *server, int port);


#endif // SERVER_H

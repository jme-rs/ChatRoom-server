#include "server.h"
#include "server_helper.h"


static handler_f server_get_handler(SERVER *server, char *path_name);
static handler_f server_get_wildcard_handler(SERVER *server);
static handler_f server_get_404_handler(SERVER *server);
static int
server_default_404_handler(int fd, sqlite3 *db, const HTTP_REQUEST *);
static void server_fork(SERVER *server, int fd, sqlite3 *db);
static void server_create_thread(void *arg);


// SERVER のインスタンスを作成
SERVER server_create()
{
    SERVER server;
    server.path_count = 0;

    return server;
}


// パスについてのハンドラを追加
void server_set_path(SERVER *server, char *path_name, handler_f handler)
{
    // パスの数が MAX_PATH を超えたらエラーを出力して終了
    if (MAX_PATH <= server->path_count) {
        fprintf(stderr, "server_add_path: MAX_PATH exceeded\n");
        exit(EXIT_FAILURE);
    }

    server->paths[server->path_count].path_name = path_name;
    server->paths[server->path_count].handler   = handler;
    server->path_count++;
}


// パスが登録されていたらそのハンドラを返す
// `handler_f` or `NULL`
static handler_f server_get_handler(SERVER *server, char *path_name)
{
    for (int i = 0; i < server->path_count; i++) {
        if (strcmp(path_name, server->paths[i].path_name) == 0)
            return server->paths[i].handler;
    }

    return NULL;
}


// ワイルドカードのハンドラを返す
// `handler_f` or `NULL`
static handler_f server_get_wildcard_handler(SERVER *server)
{
    for (int i = 0; i < server->path_count; i++) {
        if (strcmp("/*", server->paths[i].path_name) == 0)
            return server->paths[i].handler;
    }

    return NULL;
}


// 404 ハンドラが登録されていたらそのハンドラを返す
// ない場合はデフォルトの 404 ハンドラを返す
static handler_f server_get_404_handler(SERVER *server)
{
    for (int i = 0; i < server->path_count; i++) {
        if (strcmp("404", server->paths[i].path_name) == 0)
            return server->paths[i].handler;
    }

    return server_default_404_handler;
}


// デフォルトの 404 ハンドラ
static int
server_default_404_handler(int fd, sqlite3 *db, const HTTP_REQUEST *request)
{
    // HTTP レスポンスの作成
    HTTP_RESPONSE response = { .status = 404, .body = "" };

    // 送信
    int ret = send_response(fd, &response);

    return ret;
}


// サーバを起動
void server_start(SERVER *server, int port)
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

    // HTTP サーバを生成
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
                server_fork(server, newfd, db);
                close(newfd);
                goto exit_process; // 子プロセスは終了
            }

            // parent
            default: {
                close(newfd);
                break;
            }
        }
    }

exit_process: // 終了処理
    close(sockfd);
    sqlite3_close(db);
}


// fork された子プロセスの処理
static void server_fork(SERVER *server, int fd, sqlite3 *db)
{
    char *recv_buf = (char *) malloc(BUFSIZ);

    // 受信
    ssize_t receive_n = recv(fd, recv_buf, BUFSIZ, 0);
    if (receive_n < 0) {
        perror("recv");
        return;
    }

    // リクエストを解析
    const HTTP_REQUEST request = parse_request(recv_buf);

    // パスに対応するハンドラを取得
    handler_f handler;
    (handler = server_get_handler(server, (char *) request.path)) != NULL
        || (handler = server_get_wildcard_handler(server)) != NULL
        || (handler = server_get_404_handler(server));

    // ハンドラを実行
    int ret = handler(fd, db, &request);
    if (ret < 0)
        fprintf(stderr, "handler failed: %s\n", request.path);

    free(recv_buf);
}


// スレッドバージョン
void server_start_thread(SERVER *server, int port)
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

    // HTTP サーバを生成
    printf("Server listening on port %d...\n\n", port);

    while (true) {
        int newfd = accept_connection(sockfd);
        if (newfd == -1)
            break;

        // 引数用の構造体を作成
        SERVER_THREAD_ARG *server_thread_arg
            = (SERVER_THREAD_ARG *) malloc(sizeof(SERVER_THREAD_ARG));
        server_thread_arg->server = server;
        server_thread_arg->fd     = newfd;
        server_thread_arg->db     = db;

        // スレッドを作成
        pthread_t tid;
        pthread_create(&tid,
                       NULL,
                       (void *) server_create_thread,
                       (void *) server_thread_arg);
    }

    close(sockfd);
    sqlite3_close(db);
}


// 作られたスレッド
static void server_create_thread(void *arg)
{
    // 引数を取り出す
    SERVER_THREAD_ARG *server_thread_arg = (SERVER_THREAD_ARG *) arg;

    SERVER  *server = server_thread_arg->server;
    int      fd     = server_thread_arg->fd;
    sqlite3 *db     = server_thread_arg->db;

    printf("Thread created\n");

    char *recv_buf = (char *) malloc(BUFSIZ);

    // 受信
    ssize_t receive_n = recv(fd, recv_buf, BUFSIZ, 0);
    if (receive_n < 0) {
        perror("recv");
        return;
    }

    // リクエストを解析
    const HTTP_REQUEST request = parse_request(recv_buf);

    // パスに対応するハンドラを取得
    handler_f handler;
    (handler = server_get_handler(server, (char *) request.path)) != NULL
        || (handler = server_get_wildcard_handler(server)) != NULL
        || (handler = server_get_404_handler(server));

    // ハンドラを実行
    int ret = handler(fd, db, &request);
    if (ret < 0)
        fprintf(stderr, "handler failed: %s\n", request.path);

    free(recv_buf);
    close(fd);
    free(server_thread_arg); // スレッド終了時に引数用の構造体を解放
}

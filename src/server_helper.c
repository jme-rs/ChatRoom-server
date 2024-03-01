#include "server_helper.h"


int get_socket()
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    return sockfd;
}


int bind_socket(int sockfd, int port)
{
    struct sockaddr_in serv_addr;

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return -1;
    }

    return 0;
}


int accept_connection(int sockfd)
{
    struct sockaddr_in cli_addr;

    int clilen = sizeof(cli_addr);
    int newfd
        = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen);

    if (newfd < 0) {
        perror("accept");
        return -1;
    }

    return newfd;
}


int get_server_socket(int port)
{
    int sockfd = get_socket();
    if (sockfd < 0)
        return -1;

    if (bind_socket(sockfd, port) < 0) {
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}


// リクエストをパース
HTTP_REQUEST parse_request(char *request)
{
    // メソッドの特定
    char *saveptr;                                   // strtok_r 用
    char *method = strtok_r(request, " ", &saveptr); // "GET" or "POST"

    // パスの特定
    char *path = strtok_r(NULL, " ", &saveptr);

    printf("parse_request:\n"
           "\tmethod: %s\n"
           "\tpath: %s\n",
           method,
           path);

    // body を取得
    char *body;
    while ((body = strtok_r(NULL, "\n", &saveptr)) != NULL) {
        if (strlen(body) == 1)
            break;
    }
    body = strtok_r(NULL, "\0", &saveptr);

    HTTP_REQUEST req
        = { (const char *) method, (const char *) path, (const char *) body };

    return req;
}


// レスポンスを送信
int send_response(int fd, HTTP_RESPONSE *response)
{
    size_t status_len = 32;
    size_t body_len   = strlen(response->body);
    char  *send_buf   = (char *) malloc(status_len + body_len);

    sprintf(send_buf, "HTTP/1.1 %d\n\n%s", response->status, response->body);

    ssize_t send_n = send(fd, send_buf, strlen(send_buf), 0);
    if (send_n < 0) {
        perror("send");
        fprintf(stderr, "send_response");
        return -1;
    }

    printf("send_response:\n"
           "\tstatus: %d\n",
           response->status);

    free(send_buf);
    return 0;
}

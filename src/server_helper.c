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


// 長い文字列を分割して送信
// 未使用
ssize_t div_and_send(int fd, char *buf)
{
    if (buf == NULL)
        return 0;

    int len = strlen(buf);
    int n   = len / BUFSIZ;
    int r   = len % BUFSIZ;

    for (int i = 0; i < n - 1; i++) {
        if (send(fd, buf + i * BUFSIZ, BUFSIZ, 0) < 0) {
            perror("send");
            return -1;
        }
    }

    if (r > 0) {
        if (send(fd, buf + (n - 1) * BUFSIZ, r, 0) < 0) {
            perror("send");
            return -1;
        }
    }

    return 0;
}

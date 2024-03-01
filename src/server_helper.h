#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H


#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


typedef struct {
    const char *method;
    const char *path;
    const char *body;
} HTTP_REQUEST;

typedef struct {
    int   status;
    char *body;
} HTTP_RESPONSE;


int          get_socket();
int          bind_socket(int sockfd, int port);
int          accept_connection(int sockfd);
int          get_server_socket(int port);
HTTP_REQUEST parse_request(char *request);
int          send_response(int fd, HTTP_RESPONSE *response);


#endif // SERVER_HELPER_H

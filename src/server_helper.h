#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H


#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int     get_socket();
int     bind_socket(int sockfd, int port);
int     accept_connection(int sockfd);
int     get_server_socket(int port);
ssize_t div_and_send(int fd, char *buf);


#endif // SERVER_HELPER_H

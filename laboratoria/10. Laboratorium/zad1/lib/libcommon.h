#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "libprint.h"

#define LOCALHOST "127.0.0.1"
#define BOARD_FIELDS_COUNT 9

typedef enum msg_type {
    PING

} msg_type;

typedef union msg_data {
    char board[BOARD_FIELDS_COUNT];
    int move;

} msg_data;

typedef struct message {
    msg_type type;
    msg_data data;
} message;

typedef struct client {
    char* nickname;
    int socket_fd;
    bool is_available;
} client;

char* get_input_string(int *i, int argc, char* argv[], char* msg);
int get_input_num(int *i, int argc, char* argv[], char* msg);
void free_args(int arg_count, ...);
int send_message(int socket_fd, message msg);

#endif // COMMON_H

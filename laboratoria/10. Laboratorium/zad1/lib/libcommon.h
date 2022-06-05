#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "libprint.h"

#define MAX_NICKNAME_LENGTH 32

#define catch_perror(expr)             \
    catch_error(expr, strerror(errno)) \

#define catch_error(expr, err_msg)                                             \
    ({                                                                         \
        typeof(expr) __ret_val = expr;                                         \
        if (__ret_val == -1) {                                                 \
            cerror("%s:%d "#expr" failed: %s\n", __FILE__, __LINE__, err_msg); \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        __ret_val;                                                             \
    })

#define exit_error(err_msg)                                \
    ({                                                     \
        cerror("%s:%d %s\n", __FILE__, __LINE__, err_msg); \
        exit(EXIT_FAILURE);                                \
    })


typedef enum msg_type {
    // Server related message types
    PING,
    DISCONNECT,
    SERVER_FULL,
    INIT,
    // Game related message types
    STATE,
    MOVE,
    WAIT,
    WON,
    LOST, // TODO - ?
    DRAW, // TODO - ?
    USERNAME_TAKEN
} msg_type;

typedef struct game_state {
    char curr_turn_symbol;
    char board[9];
} game_state;

typedef union msg_data {
    struct {
        char nickname[MAX_NICKNAME_LENGTH];
        char symbol;  // a symbol assigned to the user (either O or X)
    } user;
    int move_id;  // a number identifying the user move (corresponding to the particular board field
    game_state state;
    char winner_symbol;  // a symbol of the winner player
} msg_data;

typedef struct message {
    msg_type type;
    msg_data data;
} message;

char* get_input_string(int *i, int argc, char* argv[], char* msg);
void free_args(int arg_count, ...);
int get_input_num(int *i, int argc, char* argv[], char* msg);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));

#endif // COMMON_H

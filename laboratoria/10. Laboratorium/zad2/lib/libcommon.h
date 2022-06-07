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
#include "libcprint.h"

#define MIN_NICKNAME_LENGTH 3
#define MAX_NICKNAME_LENGTH 32
#define PLAYER_SYMBOLS "OX"

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
    REGISTER,
    SERVER_FULL,
    USERNAME_TAKEN,
    WAIT_OPPONENT,  // A client is waiting for an opponent
    OPPONENT_DISCONNECTED,
    // Game related message types
    GAME_STARTED,
    GAME_STATE,
    GAME_RESULT,
    MOVE
} msg_type;

typedef struct game_state {
    char curr_turn_symbol;
    char board[9];
    int remaining_moves;
} game_state;

typedef enum game_result {
    WON,
    LOST,
    DRAW,
    PLAYING
} game_result;

typedef struct game_start {
    char opponent[MAX_NICKNAME_LENGTH];  // Opponent nickname
    char symbol;  // a symbol assigned to the client (either O or X)
} game_start;

typedef union msg_data {
    int move_id;  // a number identifying the client move (corresponding to the particular board field
    char nickname[MAX_NICKNAME_LENGTH];
    game_start game_start;
    game_state game_state;
    game_result game_result;
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

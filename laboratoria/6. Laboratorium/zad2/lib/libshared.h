#ifndef ZAD2_LIBSHARED_H
#define ZAD2_LIBSHARED_H

#include <time.h>

#define MSG_SEP "-"
#define SERVER_QUEUE_PATH "/server"

#define MAX_CLIENT_MSG_COUNT 10 // Max is 10
#define MAX_SERVER_MSG_COUNT 10 // Max is 10
#define MAX_BODY_LENGTH 1024
#define MAX_MSG_TOTAL_LENGTH MAX_BODY_LENGTH + 58 // +58 for other parameters sent with message

typedef struct msg_buf {
    int type;
    char body[MAX_BODY_LENGTH];
    int sender_id;
    int receiver_id;
    time_t send_time;
} msg_buf;

typedef enum msg_type {
    STOP = 1,
    LIST = 2,
    ALL = 3,
    ONE = 4,
    INIT = 5
} msg_type;

msg_buf *parse_msg(char* msg_str);
void create_msg_str(msg_buf msg, char* buff);
int create_queue(char* name, int max_count);
int send_msg(int queue_fd, msg_buf msg_obj, int type, char* error_msg);

#endif //ZAD2_LIBSHARED_H

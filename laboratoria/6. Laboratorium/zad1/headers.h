#ifndef ZAD1_SHARED_H
#define ZAD1_SHARED_H

#include <time.h>

#define MAX_MSG_LENGTH 1024

typedef struct msg_buf {
    long type;
    char body[MAX_MSG_LENGTH];
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

const int MSG_SIZE = sizeof(msg_buf) - sizeof(long);

#endif //ZAD1_SHARED_H

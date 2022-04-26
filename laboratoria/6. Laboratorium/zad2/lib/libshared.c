#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include "libshared.h"


static long parse_long(char* msg_str);
static int parse_string(char* target, char* msg_str);


msg_buf *parse_msg(char* msg_str) {
    char msg_cp[MAX_MSG_TOTAL_LENGTH];
    strcpy(msg_cp, msg_str);
    // <type>-<body>-<sender_id>-<receiver_id>-<send_time>
    msg_buf *msg = calloc(1, sizeof(msg_buf));
    if ((msg->type = (int) parse_long(msg_cp)) == -1
        || parse_string(msg->body, msg_cp) == -1
        || (msg->sender_id = (int) parse_long(msg_cp)) == -1
        || (msg->receiver_id = (int) parse_long(msg_cp)) == -1
        || (msg->send_time = parse_long(msg_cp)) == -1) {
        fprintf(stderr, "Something went wrong while parsing a message string");
        free(msg);
        return NULL;
    }

    return msg;
}

void create_msg_str(msg_buf msg, char* buff) {
    char temp[MAX_MSG_TOTAL_LENGTH];
    sprintf(temp, "%d%s%s%s%d%s%d%s%ld",
            msg.type,
            MSG_SEP,
            strlen(msg.body) ? msg.body : NULL,
            MSG_SEP,
            msg.sender_id,
            MSG_SEP,
            msg.receiver_id,
            MSG_SEP,
            msg.send_time
    );

    strcpy(buff, temp);
    buff[strlen(buff)] = '\0';
}

int create_queue(char* name, int max_count) {
    struct mq_attr attr = {
            .mq_msgsize = MAX_MSG_TOTAL_LENGTH,
            .mq_maxmsg = max_count
    };

    mqd_t queue_fd = mq_open(name, O_RDONLY | O_CREAT, 0666, &attr);
    if (queue_fd == -1) perror("Unable to create a queue.\n");
    return queue_fd;
}

int send_msg(int queue_fd, msg_buf msg_obj, int type, char* error_msg) {
    char msg[MAX_MSG_TOTAL_LENGTH];
    create_msg_str(msg_obj, msg);

    if (mq_send(queue_fd, msg, strlen(msg), type) == -1) {
        perror(error_msg);
        return -1;
    }
    return 0;
}

static long parse_long(char* msg_str) {
    char* rest;
    char* num_str;
    if (!(num_str = strtok_r(msg_str, MSG_SEP, &rest))) {
        fprintf(stderr, "Unable to parse long. There are no more tokens in a string.\n");
        return -1;
    }

    long num = strtol(num_str, NULL, 10);
    strcpy(msg_str, rest);
    if (num == 0 && errno) {
        perror("Unable to convert string to the number.\n");
        return -1;
    }

    return num;
}

static int parse_string(char* target, char* msg_str) {
    char* rest;
    char* body;
    if (!(body = strtok_r(msg_str, MSG_SEP, &rest))) {
        fprintf(stderr, "Unable to parse string. There are no more tokens in a string.\n");
        return -1;
    }
    strcpy(target, body);
    strcpy(msg_str, rest);
    return 0;
}

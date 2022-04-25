#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include "lib/libshared.h"


const char* LIST_CMD = "LIST";
const char* ALL_CMD = "2ALL";
const char* ONE_CMD = "2ONE";
const char* STOP_CMD = "STOP";

int CLIENT_QUEUE_FD = 0;
int SERVER_QUEUE_FD = 0;
int ID = -1;
pid_t CHILD_PID = -1;
pid_t PARENT_PID;

void exit_handler(void);
void sigint_handler(int sig_no);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int setup_queues(void);
int send_INIT();
void send_STOP();
int send_ONE(int receiver_id, char* body);
int send_ALL(char* body);
int send_LIST(void);
int run(void);
int handle_queue_msg(void);
int handle_queue_input(void);
char* get_input_string(char* msg);
char* get_msg_body(char* rest);
int get_receiver_id(char* rest);
int show_msg(msg_buf *msg);
int get_queue_name(char *name);
int send_msg_to_server(msg_buf msg);
msg_buf *receive_msg_from_server(int type);


int main(void) {
    PARENT_PID = getpid();
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    if (atexit(exit_handler) == -1) {
        perror("Unable to set the exit handler.\n");
        return 1;
    }
    if (set_sa_handler(SIGINT, 0, send_STOP)) return 3;
    if (setup_queues() == -1) return 2;
    if (send_INIT() == -1) {
        fprintf(stderr, "Unable to initialize a client.\n");
        return 4;
    }

    // The run function will only return if there was some error
    run();
    return 5;
}


int get_queue_name(char *name) {
    char* home_path = getenv("HOME");
    if (!home_path) {
        fprintf(stderr, "Unable to get HOME path value.\n");
        return -1;
    }

    char home_path_cp[MAX_BODY_LENGTH];
    strcpy(home_path_cp, home_path);
    char* rest = "";
    if (!strtok_r(home_path_cp, "/", &rest)) {
        fprintf(stderr, " There are no more tokens in a string.\n");
        return -1;
    }
    sprintf(name, "/%s%d", rest, getpid());

    return 0;
}

void exit_handler(void) {
    if (getpid() == PARENT_PID) {
        if (CHILD_PID > 0) kill(CHILD_PID, SIGKILL);

        if (mq_close(SERVER_QUEUE_FD) == -1) {
            perror("Unable to close a server queue.\n");
            exit(1);
        }
        if (mq_close(CLIENT_QUEUE_FD) == -1) {
            perror("Unable to close a client queue.\n");
            exit(1);
        }
        char name[MAX_BODY_LENGTH];
        if (get_queue_name(name) == -1) exit(1);
        if (mq_unlink(name) == 0) printf("Removed old message queue '%s' from the system.\n", name);
        else if (errno != ENOENT) {
            perror("Unable to unlink the old message queue.\n");
            exit(1);
        }
        exit(0);
    }
}

int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int)) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_handler = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        perror("Action cannot be set for the signal.\n");
        return -1;
    }

    return 0;
}

int setup_queues(void) {
    char name[MAX_BODY_LENGTH];
    if (get_queue_name(name) == -1) return -1;

    printf("Client queue name '%s'\n", name);
    if ((CLIENT_QUEUE_FD = create_queue(name, MAX_CLIENT_MSG_COUNT)) == -1) return -1;
    if ((SERVER_QUEUE_FD = mq_open(SERVER_QUEUE_PATH, O_WRONLY)) == -1) {
        perror("Unable to open a server queue.\n");
        return -1;
    }

    return 0;
}

int send_msg_to_server(msg_buf msg) {
    return send_msg(SERVER_QUEUE_FD, msg, msg.type, "Unable to send a message to a server.\n");
}

msg_buf *receive_msg_from_server(int type) {
    char msg_str[MAX_MSG_TOTAL_LENGTH];
    unsigned int type_ = ((unsigned int) type);
    if (mq_receive(CLIENT_QUEUE_FD, msg_str, MAX_MSG_TOTAL_LENGTH, type < 0 ? NULL : &type_) == -1) {
        perror("Unable to receive a message from the server queue.\n");
        return NULL;
    }

    return parse_msg(msg_str);
}

int send_INIT() {
    char name[MAX_BODY_LENGTH];
    if (get_queue_name(name) == -1) return -1;

    msg_buf msg = {
            .type = INIT,
    };
    sprintf(msg.body, "%s", name);
    if (send_msg_to_server(msg) == -1) return -1;

    msg_buf *response;
    if (!(response = receive_msg_from_server(INIT))) return -1;

    ID = (int) strtol(response->body, NULL, 10);
    if (ID == 0 && errno) {
        perror("Unable to convert a message body into a number.\n");
        free(response);
        return -1;
    }

    printf("Client was assigned '%d' id.\n\n", ID);
    free(response);

    return 0;
}

void send_STOP() {
    if (getpid() != PARENT_PID) kill(PARENT_PID, SIGINT);
    else {
        puts("\nSending STOP message...\n");
        msg_buf msg = {
                .type = STOP,
                .sender_id = ID
        };
        int status = send_msg_to_server(msg) == -1 ? 1 : 0;
        if (status == 0) {
            printf("Client was successfully stopped.\n");
        } else {
            fprintf(stderr, "Something went wrong while trying to stop a client.\n");
        }
        exit(status);
    }
}

int send_ONE(int receiver_id, char* body) {
    puts("Sending 2ONE message...\n");
    msg_buf msg = {
            .type = ONE,
            .sender_id = ID,
            .receiver_id = receiver_id,
            .send_time = time(NULL)
    };
    strcpy(msg.body, body);
    return send_msg_to_server(msg);
}

int send_ALL(char* body) {
    puts("Sending 2ALL message...\n");
    msg_buf msg = {
            .type = ALL,
            .sender_id = ID,
            .send_time = time(NULL)
    };
    strcpy(msg.body, body);
    return send_msg_to_server(msg);
}

int send_LIST(void) {
    puts("Sending LIST message...\n");
    msg_buf msg = {
            .type = LIST,
            .sender_id = ID
    };
    if (send_msg_to_server(msg) == -1) return -1;

    msg_buf *response;
    if (!(response = receive_msg_from_server(LIST))) return -1;

    printf("Active clients:\n%s\n", response->body);
    free(response);
    return 0;
}

int run(void) {
    struct mq_attr attr;

    CHILD_PID = fork();
    if (CHILD_PID == -1) {
        perror("The child process could not be created.\n");
        return -1;
    }

    // Handle input in a parent process
    if (CHILD_PID == 0) {
        while (true) {
            if (handle_queue_input() == -1) return -1;
        }
        // Handle queued messages in a child process
    } else {
        while (true) {
            if (mq_getattr(CLIENT_QUEUE_FD, &attr) == -1) {
                perror("Unable to retrieve information about client queue.\n");
                return -1;
            }
            if (attr.mq_curmsgs > 0) {
                if (handle_queue_msg() == -1) exit(1);
                else printf("Enter a command:\n");
            }
        }
    }
}

int handle_queue_msg(void) {
    puts("\n");

    // Get the oldest communication from the client queue
    msg_buf *msg;
    if (!(msg = receive_msg_from_server(-1))) return -1;
    int status;

    switch (msg->type) {
        case ONE:
            printf("Received 2ONE message.\n");
            status = show_msg(msg);
            break;
        case ALL:
            printf("Received 2ALL message.\n");
            status = show_msg(msg);
            break;
        case STOP:
            printf("Received STOP message.\n");
            exit_handler();
            break;
        default:
            fprintf(stderr, "Unrecognized message type '%d'.\n", msg->type);
            status = -1;
    }

    free(msg);
    return status;
}

int handle_queue_input(void) {
    char* input = get_input_string("Enter a command:");
    while (!strlen(input)) {
        puts("Input not recognized. Please try again or stop using CTRL+C.");
        input = get_input_string("Enter a command:");
    }

    char* rest;
    char* cmd = strtok_r(input, " \0", &rest);

    if (!strcmp(cmd, LIST_CMD)) {
        return send_LIST();
    } else if (!strcmp(cmd, ALL_CMD)) {
        char* body = get_msg_body(rest);
        if (!body) {
            fprintf(stderr, "Something went wrong while sending a message to all receivers.\n");
            send_STOP();
        }
        return send_ALL(rest);
    } else if (!strcmp(cmd, ONE_CMD)) {
        int receiver_id;
        char* body;
        if ((receiver_id = get_receiver_id(rest)) < 0 || !(body = get_msg_body(rest))) {
            fprintf(stderr, "Something went wrong while sending a message to one receiver.\n");
            send_STOP();
        }
        return send_ONE(receiver_id, body);
    } else if (!strcmp(cmd, STOP_CMD)) {
        send_STOP();
    }

    fprintf(stderr, "Command '%s' is not recognized.\n", cmd);
    send_STOP();
    return -1;
}

char* get_input_string(char* msg) {
    printf("%s\n", msg);
    char* line = "";
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

char* get_msg_body(char* rest) {
    char* body;
    if ((body = strtok(rest, "\n\0")) == NULL) {
        fprintf(stderr, "Unable to get message body. There are no more tokens in a string.\n");
        return NULL;
    }
    return body;
}

int get_receiver_id(char* rest) {
    char* receiver_id_str;
    char* rest_ = "";
    if ((receiver_id_str = strtok_r(rest, " \0", &rest_)) == NULL) {
        fprintf(stderr, "Unable to get receiver_id. There are no more tokens in a string.\n");
        return -1;
    }

    int receiver_id = (int) strtol(receiver_id_str, NULL, 10);
    if (receiver_id < 0 || errno) {
        fprintf(stderr, "Invalid receiver id. Expected a non-negative integer, got '%s'\n", receiver_id_str);
        return -1;
    }

    strcpy(rest, rest_);
    return receiver_id;
}

int show_msg(msg_buf *msg) {
    time_t send_time = (time_t) msg->send_time;
    struct tm *local_time = localtime(&send_time);
    if (!local_time) {
        perror("Unable to get a local time.\n");
        return -1;
    }

    printf("Sender id: %d\nMessage type: %d\nMessage body:\n%s\nTime of sending %d-%02d-%02d %02d:%02d:%02d\n\n",
           msg->sender_id,
           msg->type,
           msg->body,
           local_time->tm_year + 1900,
           local_time->tm_mon + 1,
           local_time->tm_mday,
           local_time->tm_hour,
           local_time->tm_min,
           local_time->tm_sec
    );
    return 0;
}

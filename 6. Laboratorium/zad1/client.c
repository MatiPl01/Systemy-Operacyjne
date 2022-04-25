#include "headers.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


const char PROJ_ID = 'C';
const char* LIST_CMD = "LIST";
const char* ALL_CMD = "2ALL";
const char* ONE_CMD = "2ONE";
const char* STOP_CMD = "STOP";

int CLIENT_KEY = 0;
int CLIENT_QUEUE = 0;
int SERVER_QUEUE = 0;
int ID = -1;
pid_t CHILD_PID = -1;

int handle_exit(void);
void sigint_handler(int sig_no);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int setup_queues(void);
int send_INIT();
int send_STOP();
int send_ONE(int receiver_id, char* body);
int send_ALL(char* body);
int send_LIST(void);
int run(void);
int handle_queue_msg(void);
int handle_queue_input(void);
char* get_input_string(char* msg);
char* get_msg_body(char* rest);
int get_receiver_id(char* rest);
int show_msg(msg_buf msg);


int main(void) {
    atexit((void (*)(void)) handle_exit);
    if (setup_queues() == -1) return 1;
    if (set_sa_handler(SIGINT, 0, (void (*)(int)) send_STOP)) return 2;
    if (send_INIT() == -1) {
        fprintf(stderr, "Unable to initialize a client.\n");
        return 3;
    }

    // The run function will only return if there was some error
    run();
    return 4;
}


int handle_exit(void) {
    if (msgctl(CLIENT_QUEUE, IPC_RMID, NULL) == -1) {
        perror("Unable to remove queue.\n");
        return -1;
    }
    return 0;
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
    char* home_path = getenv("HOME");

    key_t server_key = ftok(home_path, PROJ_ID);
    if ((SERVER_QUEUE = msgget(server_key, 0)) == -1) {
        perror("Unable to get a server queue identifier.\n");
        return -1;
    }
    printf("Server queue with key '%d' was found.\n", server_key);

    CLIENT_KEY = ftok(home_path, getpid());
    if ((CLIENT_QUEUE = msgget(CLIENT_KEY, IPC_CREAT | 0666)) == -1) {
        perror("Unable to create a client queue.\n");
        return -1;
    }
    printf("Client queue with '%d' key was successfully created.\n", CLIENT_KEY);

    return 0;
}

int send_msg(msg_buf msg) {
    if (msgsnd(SERVER_QUEUE, &msg, MSG_SIZE, 0) == -1) {
        perror("Unable to send a message to the server queue.\n");
        return -1;
    }
    return 0;
}

int send_INIT() {
    msg_buf msg = {
            .type = INIT,
    };
    sprintf(msg.body, "%d", CLIENT_KEY);
    if (send_msg(msg) == -1) return -1;

    msg_buf response;
    if (msgrcv(CLIENT_QUEUE, &response, MSG_SIZE, INIT, 0) == -1) {
        perror("Unable to receive a message from the client queue.\n");
        return -1;
    }

    ID = (int) strtol(response.body, NULL, 10);
    printf("Client was assigned '%d' id.\n\n", ID);

    return 0;
}

int send_STOP() {
    puts("Sending STOP message...\n");
    msg_buf msg = {
            .type = STOP,
            .sender_id = ID
    };
    int status = send_msg(msg) == -1 ? 1 : 0;
    if (CHILD_PID > 0) kill(CHILD_PID, SIGKILL);
    if (status == 0) {
        printf("Client was successfully stopped.\n");
    } else {
        fprintf(stderr, "Something went wrong while trying to stop a client.\n");
    }
    exit(status);
}

int send_ONE(int receiver_id, char* body) {
    puts("Sending 2ONE message...\n");
    msg_buf msg = {
            .type = ONE,
            .sender_id = ID,
            .receiver_id = receiver_id,
    };
    strcpy(msg.body, body);
    return send_msg(msg);
}

int send_ALL(char* body) {
    puts("Sending 2ALL message...\n");
    msg_buf msg = {
            .type = ALL,
            .sender_id = ID
    };
    strcpy(msg.body, body);
    return send_msg(msg);
}

int send_LIST(void) {
    puts("Sending LIST message...\n");
    msg_buf msg = {
            .type = LIST,
            .sender_id = ID
    };
    if (send_msg(msg) == -1) return -1;

    msg_buf response;
    if (msgrcv(CLIENT_QUEUE, &response, MSG_SIZE, LIST, 0) == -1) {
        perror("Unable to receive a list of active clients from the server.\n");
        return -1;
    }

    printf("Active clients:\n%s\n", response.body);
    return 0;
}

int run(void) {
    struct msqid_ds queue_stat;

    CHILD_PID = fork();
    if (CHILD_PID == -1) {
        perror("The child process could not be created.\n");
        return -1;
    }

    // Handle queued messages in a child process
    if (CHILD_PID == 0) {
        while (true) {
            if (msgctl(CLIENT_QUEUE, IPC_STAT, &queue_stat) == -1) {
                perror("Unable to retrieve information about client queue.\n");
                return -1;
            }
            if (queue_stat.msg_qnum) {
                if (handle_queue_msg() == -1) exit(1);
                else printf("Enter a command:\n");
            }
        }
        // Handle input in a parent process
    } else {
        while (true) {
            if (handle_queue_input() == -1) return -1;
        }
    }
}

int handle_queue_msg(void) {
    puts("\n");
    msg_buf msg;

    // Get the oldest communication from the client queue
    if (msgrcv(CLIENT_QUEUE, &msg, MSG_SIZE, 0, 0) == -1) {
        perror("Unable to get communication from the client queue.\n");
        return -1;
    }

    switch (msg.type) {
        case ONE:
            printf("Received 2ONE message.\n");
            return show_msg(msg);
        case ALL:
            printf("Received 2ALL message.\n");
            return show_msg(msg);
        default:
            fprintf(stderr, "Unrecognized message type '%ld'.\n", msg.type);
            return -1;
    }
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
            return -1;
        }
        return send_ALL(rest);
    } else if (!strcmp(cmd, ONE_CMD)) {
        int receiver_id;
        char* body;
        if ((receiver_id = get_receiver_id(rest)) < 0 || !(body = get_msg_body(rest))) {
            fprintf(stderr, "Something went wrong while sending a message to one receiver.\n");
            send_STOP();
            return -1;
        }
        return send_ONE(receiver_id, body);
    } else if (!strcmp(cmd, STOP_CMD)) {
        return send_STOP();
    }

    fprintf(stderr, "Command '%s' is not recognized.\n", cmd);
    return send_STOP();
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

int show_msg(msg_buf msg) {
    time_t send_time = msg.send_time;
    struct tm *local_time = localtime(&send_time);
    if (!local_time) {
        perror("Unable to get a local time.\n");
        return -1;
    }

    printf("Sender id: %d\nMessage type: %ld\nMessage body:\n%s\nTime of sending %d-%02d-%02d %02d:%02d:%02d\n\n",
           msg.sender_id,
           msg.type,
           msg.body,
           local_time->tm_year + 1900,
           local_time->tm_mon + 1,
           local_time->tm_mday,
           local_time->tm_hour,
           local_time->tm_min,
           local_time->tm_sec
    );
    return 0;
}

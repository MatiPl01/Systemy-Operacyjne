#include "headers.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

const char PROJ_ID = 'C';
const char* RES_FILE_PATH = "results.txt";

int MAX_NO_CLIENTS = 100;
int CLIENT_QUEUES[MAX_NO_CLIENTS] = { 0 };

key_t generate_key(void);
int create_queue(key_t *key);
int listen(int queue_id);
int receive_msg(int queue_id);
int handle_msg(msg_buf msg);
int save_msg(msg_buf msg);
int init_client(char* body);
int stop_client(int client_id);
int send_to_one(int receiver_id, int sender_id, char* body);
int send_to_all(int sender_id, char* body);
void list_all_active_clients(void);
struct tm* get_time(void);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
void exit_handler(void);
void sigint_handler(int sig_no);


int main(void) {
    if (atexit(exit_handler) == -1) {
        perror("Unable to set the exit handler.\n");
        return 1;
    }
    if (set_sa_handler(SIGINT, 0, sigint_handler) == -1) return -1;

    key_t key;
    int queue_id = create_queue(&key);
    if (queue_id == -1) return 3;
    if (listen(queue_id) == -1) return 4;
    return 0;
}


key_t generate_key() {
    char* home_path = getenv("HOME");
    key_t key = ftok(home_path, PROJ_ID);
    if (key == -1) {
        perror("Unable to generate a key.\n");
        return -1;
    }
    return key;
}

int create_queue(key_t *key) {
    if ((*key = generate_key()) == -1) return -1;
    int queue_id = msgget(*key, IPC_CREAT | IPC_EXCL | 0666);
    if (queue_id == -1) {
        perror("Unable to create a message queue.\n");
        return -1;
    }
    return queue_id;
}

int listen(int queue_id) {
    while (true) {
        if (receive_msg(queue_id) == -1) return -1;
    }
}

int receive_msg(int queue_id) {
    msg_buf msg;
    size_t no_bytes = msgrcv(queue_id, &msg, MSG_SIZE, -(INIT + 1), 0);
    if (no_bytes == -1) {
        perror("Error while trying to receive a message.\n");
        return -1;
    }
    return handle_msg(msg);
}

int handle_msg(msg_buf msg) {
    if (save_msg(msg) == -1) return -1;

    switch (msg.type) {
        case INIT:
            printf("Received INIT from %d client.\n", msg.sender_id);
            return init_client(msg.body);
        case STOP:
            printf("Received STOP from %d client.\n", msg.sender_id);
            return stop_client(msg.receiver_id);
        case ALL:
            printf("Received ALL from %d client.\n", msg.sender_id);
            return send_to_all(msg.sender_id, msg.body);
        case ONE:
            printf("Received ONE from %d client.\n", msg.sender_id);
            return send_to_one(msg.receiver_id, msg.sender_id, msg.body);
        case LIST:
            printf("Received LIST from %d client.\n", msg.sender_id);
            return list_all_clients();
        default:
            fprintf(stderr, "Unrecognized message type '%ld'.\n", msg.type);
            return -1;
    }
}

struct tm* get_time(void) {
    time_t time_now = time(NULL);
    struct tm *local_time = localtime(&time_now);
    if (!local_time) {
        perror("Unable to get a local time.\n");
        return NULL;
    }
    return local_time;
}

int save_msg(msg_buf msg) {
    FILE *f_ptr = fopen(RES_FILE_PATH, "a");
    if (!f_ptr) {
        perror("Failed to open a results file.\n");
        return -1;
    }

    struct tm *local_time = get_time();

    if (fprintf(f_ptr, "%d-%02d-%02d %02d:%02d:%02d\nClient id: %d\nMessage:\n%s\n\n",
                local_time->tm_year + 1900,
                local_time->tm_mon + 1,
                local_time->tm_mday,
                local_time->tm_hour,
                local_time->tm_min,
                local_time->tm_sec,
                msg.sender_id,
                msg.body
    ) < 0) {
        fprintf(stderr, "Unable to write data to a file.\n");
        return -1;
    }

    fclose(f_ptr);

    return 0;
}

int get_free_id(void) {
    for (int i = 0; i < MAX_NO_CLIENTS; i++) {
        if (!CLIENT_QUEUES[i]) return i;
    }
    return -1;
}

int init_client(char* body) {
    int id = get_free_id();
    if (id == -1) {
        fprintf(stderr, "Impossible to initialize a new client. The number of possible clients (%d) was exceeded.\n", MAX_NO_CLIENTS);
        return -1;
    }

    key_t key = (key_t) strtol(body, NULL, 10);
    if ((CLIENT_QUEUES[id] = msgget(key, 0)) == -1) {
        perror("Unable to get a client queue identifier.\n");
        return -1;
    }

    msg_buf msg = {
        .type = INIT;
    };
    sprintf(msg.body, "%d", id);

    if (msgsnd(CLIENT_QUEUES[id], &msg, MSG_SIZE, 0) == -1) {
        perror("Unable to send a response to the client.\n");
        return -1;
    }

    return 0;
}

int stop_client(int client_id) {
    if (!CLIENT_QUEUES[client_id]) {
        fprintf(stderr, "Cannot stop a client. There is no client with id '%d'.\n", client_id);
    }
    CLIENT_QUEUES[client_id] = 0;
    return 0;
}

int send_to_one(int receiver_id, int sender_id, char* body) {
    if (!CLIENT_QUEUES[receiver_id]) {
        fprintf(stderr, "Cannot send a message to a client. There is no client with id '%d'.\n", receiver_id);
        return -1;
    }

    msg_buf msg = {
        .type = ONE,
        .sender_id = sender_id,
        .receiver_id = receiver_id;
    };
    strcpy(msg.body, body);

    struct tm* msg_time = get_time();
    if (!msg_time) return -1;
    msg.send_time = msg_time;

    if (msgsnd(CLIENT_QUEUES[receiver_id], &msg, MSG_SIZE, 0) == -1) {
        perror("Unable to send a message to the receiver client.\n");
        return -1;
    }

    return 0;
}

int send_to_all(int sender_id, char* body) {
    for (int receiver_id = 0; receiver_id < MAX_NO_CLIENTS; receiver_id++) {
        if (CLIENT_QUEUES[receiver_id] && receiver_id != sender_id) {
            if (send_to_one(receiver_id, sender_id, body) == -1) return -1;
        }
    }
    return 0;
}

void list_all_active_clients(void) {
    printf("All active clients:\n");
    for (int receiver_id; receiver_id < MAX_NO_CLIENTS; receiver_id++) {
        if (CLIENT_QUEUES[receiver_id]) printf("%d\n", receiver_id);
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

void exit_handler(void) {
    msg_buf msg = { .type = STOP };
    for (int receiver_id = 0; receiver_id < MAX_NO_CLIENTS; receiver_id++) {
        if (!CLIENT_QUEUES[receiver_id]) {
            printf("Client %d is already stopped.\n", receiver_id);
        } else if (msgsnd(CLIENT_QUEUES[receiver_id], &msg, MSG_SIZE, 0) == -1) {
            fprintf(stderr, "Unable to send a STOP message to the '%d' client.\n", receiver_id);
        } else {
            printf("STOP message was successfully sent to the '%d' client.\n", receiver_id);
        }
    }
}

void sigint_handler(int sig_no) {
    printf("Received '%d' signal. Closing the server...\n", sig_no);
    exit(0);
}

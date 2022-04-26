#include "headers.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#define MAX_NO_CLIENTS 100

const char PROJ_ID = 'C';
const char* RES_FILE_PATH = "logs.txt";

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
int list_all_active_clients(int sender_id) ;
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
    printf("Server queue with '%d' key was successfully created.\n", key);

    // The run function will only return if there was some error
    listen(queue_id);
    return 4;
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
    int queue_id = msgget(*key, IPC_CREAT | 0666);
    if (queue_id == -1) {
        perror("Unable to create a server message queue.\n");
        return -1;
    }
    return queue_id;
}

int listen(int queue_id) {
    printf("Starting listening to clients...\n");
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
            return stop_client(msg.sender_id);
        case ALL:
            printf("Received 2ALL from %d client.\n", msg.sender_id);
            return send_to_all(msg.sender_id, msg.body);
        case ONE:
            printf("Received 2ONE from %d client.\n", msg.sender_id);
            return send_to_one(msg.receiver_id, msg.sender_id, msg.body);
        case LIST:
            printf("Received LIST from %d client.\n", msg.sender_id);
            return list_all_active_clients(msg.sender_id);
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

    if (fprintf(f_ptr, "%d-%02d-%02d %02d:%02d:%02d\nClient id: %d\nMessage type: %ld\nMessage body:\n'%s'\n\n",
                local_time->tm_year + 1900,
                local_time->tm_mon + 1,
                local_time->tm_mday,
                local_time->tm_hour,
                local_time->tm_min,
                local_time->tm_sec,
                msg.sender_id,
                msg.type,
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
    printf("Assigned '%d' id to the new client.\n", id);

    key_t client_key = (key_t) strtol(body, NULL, 10);
    if ((CLIENT_QUEUES[id] = msgget(client_key, 0)) == -1) {
        perror("Unable to get a client queue identifier.\n");
        return -1;
    }

    msg_buf msg = {
            .type = INIT
    };
    sprintf(msg.body, "%d", id);
    printf("Client queue with key '%d' was found.\n", client_key);

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

    printf("Stop message was successfully sent to '%d'.\n", client_id);
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
            .receiver_id = receiver_id,
            .send_time = time(NULL)
    };
    strcpy(msg.body, body);

    if (msgsnd(CLIENT_QUEUES[receiver_id], &msg, MSG_SIZE, 0) == -1) {
        perror("Unable to send a message to the receiver client.\n");
        return -1;
    }

    printf("Message from '%d' to '%d' was successfully sent.\n", sender_id, receiver_id);
    return 0;
}

int send_to_all(int sender_id, char* body) {
    for (int receiver_id = 0; receiver_id < MAX_NO_CLIENTS; receiver_id++) {
        if (CLIENT_QUEUES[receiver_id] && receiver_id != sender_id) {
            if (send_to_one(receiver_id, sender_id, body) == -1) return -1;
        }
    }

    printf("Message from '%d' was successfully sent to all remaining clients.\n", sender_id);
    return 0;
}

int list_all_active_clients(int sender_id) {
    printf("All active clients:\n");
    char buff[MAX_MSG_LENGTH];
    char temp[32];
    buff[0] = '\0';

    for (int receiver_id = 0; receiver_id < MAX_NO_CLIENTS; receiver_id++) {
        if (CLIENT_QUEUES[receiver_id]) {
            printf("%d\n", receiver_id);
            sprintf(temp, "%d\n", receiver_id);
            strcat(buff, temp);
        }
    }

    msg_buf msg = { .type = LIST };
    strcpy(msg.body, buff);

    if (msgsnd(CLIENT_QUEUES[sender_id], &msg, MSG_SIZE, 0) == -1) {
        perror("Unable to send a message to the receiver client.\n");
        return -1;
    }

    puts("List message was successfully sent.");
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

void exit_handler(void) {
    msg_buf msg = { .type = STOP };
    for (int receiver_id = 0; receiver_id < MAX_NO_CLIENTS; receiver_id++) {
        if (!CLIENT_QUEUES[receiver_id]) {
            continue;
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

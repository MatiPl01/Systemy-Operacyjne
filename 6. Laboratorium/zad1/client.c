#include "headers.h"
#include <stdbool.h>


const char PROJ_ID = 'C';

int CLIENT_KEY = 0;
int CLIENT_QUEUE = 0;
int SERVER_QUEUE = 0;
int ID = -1;

int remove_queue(void);
void sigint_handler(int sig_no);
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int setup_queues(void);
int send_INIT(int receiver_id);
int send_STOP(int receiver_id);
int send_ONE(int receiver_id, char* body);
int send_ALL(char* body);
int send_LIST(void);
int run(void);


int main(void) {
    atexit(remove_queue);
    if (set_sa_handler(SIGINT, 0, send_STOP)) return 1;
    if (setup_queues() == -1) return 2;
    if (run() == -1) return 3;
    return 0;
}


int remove_queue(void) {
    if (msgctl(QUEUE, IPC_RMID, NULL) == -1) {
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

    CLIENT_KEY = ftok(home_path, getpid());
    if ((CLIENT_QUEUE = msgget(client_key, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        perror("Unable to create a client queue.\n");
        return -1;
    }

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
        .body = CLIENT_KEY
    };
    if (send_msg(msg) == -1) return -1;

    msg_buf response;
    if (msgrcv(CLIENT_QUEUE, &response, MSG_SIZE, INIT, 0) == -1) {
        perror("Unable to receive a message from the client queue.\n");
        return -1;
    }

    ID = (int) strtol(response.body, NULL, 10);

    return 0;
}

int send_STOP(int receiver_id) {
    msg_buf msg = {
        .type = STOP,
        .receiver_id = receiver_id
    };
    return send_msg(msg);
}

int send_ONE(int receiver_id, char* body) {
    msg_buf msg = {
        .type = ONE,
        .sender_id = ID,
        .receiver_id = receiver_id,
    };
    strcpy(msg.body, body);
    return send_msg(msg);
}

int send_ALL(char* body) {
    msg_buf msg = {
        .type = ALL,
        .sender_id = ID
    };
    strcpy(msg.body, body);
    return send_msg(msg);
}

int send_LIST(void) {
    msg_buf msg = { .type = LIST };
    return send_msg(msg);
}

int run(void) {

}

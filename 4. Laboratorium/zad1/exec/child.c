#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define IGNORE_ACTION_NAME  "ignore"
#define MASK_ACTION_NAME    "mask"
#define PENDING_ACTION_NAME "pending"


char* get_action(int argc, char* argv[]);
int get_sig_no(int argc, char* argv[]);
int handle_action(char* action, int sig_no);
int raise_signal(int sig_no);
int is_signal_pending(int sig_no);


int main(int argc, char* argv[]) {
    printf("\tv Child process\n");

    char* action;
    int sig_no;

    if (!(action = get_action(argc, argv))) return 1;
    if ((sig_no = get_sig_no(argc, argv)) == -1) return 1;

    if (handle_action(action, sig_no) == -1) return 1;

    printf("\t^ End of a child process\n");

    return 0;
}


char* get_action(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No argument for action was specified.\n");
        return NULL;
    }

    return argv[1];
}

int get_sig_no(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "No argument for signal to listen to was specified.\n");
        return -1;
    }

    return (int) strtol(argv[2], NULL, 10);
}

int handle_action(char* action, int sig_no) {
    if (strcmp(action, IGNORE_ACTION_NAME) == 0) {
        return raise_signal(sig_no);
    }
    if (strcmp(action, MASK_ACTION_NAME) == 0) {
        return raise_signal(sig_no);
    }
    if (strcmp(action, PENDING_ACTION_NAME) == 0) {
        return is_signal_pending(sig_no);
    }

    fprintf(stderr, "Undefined action\n");
    return -1;
}

int raise_signal(int sig_no) {
    printf("Raising a signal '%d'.\n", sig_no);

    if (raise(sig_no) != 0) {
        perror("Unable to raise a signal.\n");
        return -1;
    }

    return 0;
}

int is_signal_pending(int sig_no) {
    sigset_t pending;

    // Get information about pending signals
    if (sigpending(&pending) == -1) {
        perror("Pending signals cannot be examined.\n");
        return -1;
    }

    // Check if sig_no is one of pending signals
    switch (sigismember(&pending, sig_no)) {
        case 1:
            printf("Signal %d is pending.\n", sig_no);
            break;
        case 0:
            printf("Signal %d is not pending.\n", sig_no);
            break;
        default:
            perror("Unable to check if a signal is pending in a child process.\n");
            return -1;
    }

    return 0;
}

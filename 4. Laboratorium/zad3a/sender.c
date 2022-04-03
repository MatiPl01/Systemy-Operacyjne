#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define TARGET_SIGNAL SIGUSR1
#define STOP_SIGNAL   SIGUSR2

#define SIGRT_TARGET_SIGNAL SIGRTMIN + 1
#define SIGRT_STOP_SIGNAL   SIGRTMIN + 2

#define KILL_MODE     "kill"
#define SIGQUEUE_MODE "sigqueue"
#define SIGRT_MODE    "sigrt"

int received_signals_count = 0;
int sent_signals_count = 0;
int target_signal;
int stop_signal;
char* mode = "";

char* get_input_string(int *i, int argc, char* argv[], char* msg);
int get_input_num(int *i, int argc, char* argv[], char* msg);
int send_signals(pid_t catcher_PID, int signals_count);
int send_signal(int sig_no, pid_t catcher_PID);
int set_sa_sigaction(int sig_no, int sa_flags, void (*handler)(int, siginfo_t*, void*));
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int set_up_signal_handlers(void);
void set_signal_numbers(void);
void target_signal_handler(int sig_no, siginfo_t *info, void *ucontext);
void stop_signal_handler(int sig_no);


int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    // Get settings parameters from input
    int i = 1;

    pid_t catcher_PID = (pid_t) get_input_num(&i, argc, argv, "Please provide a PID of a catcher process");
    if (catcher_PID < 1) {
        fprintf(stderr, "A PID of a catcher process should be a positive integer.\n");
        return 1;
    }

    int signals_count = get_input_num(&i, argc, argv, "Please provide a number of signals to send");
    if (signals_count < 1) {
        fprintf(stderr, "A number of signals to send should be a positive integer.\n");
        return 1;
    }

    mode = get_input_string(&i, argc, argv, "Please provide a sending signals mode");
    set_signal_numbers();

    // Display settings
    printf("Sender (PID: %d):\n", getpid());
    printf("\tSettings:\n");
    printf("\tCatcher's PID:     %d\n", catcher_PID);
    printf("\tNumber of signals: %d\n", signals_count);
    printf("\tMode:              %s\n\n", mode);

    // Set up signal handlers
    if (set_up_signal_handlers() == -1 ||
        // Send signals to the catcher
        send_signals(catcher_PID, signals_count) == -1)
    {
        if (argc < 4) free(mode);
        return 1;
    }

    pause(); // Wait for a signal from the catcher process
    printf("\nI've also finished! Thank you CATCHER for good cooperation ❤.\n");
    puts("Here are my statistics:");
    printf("Total number of signals sent:     %d\n", sent_signals_count);
    printf("Total number of signals received: %d\n", received_signals_count);

    return 0;
}


char* get_input_string(int *i, int argc, char* argv[], char* msg) {
    // If an argument was provided, return this argument
    if (*i < argc) return argv[(*i)++];

    // Otherwise, get an input from a user
    printf("%s\n>>> ", msg);
    char* line = "";
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

int get_input_num(int *i, int argc, char* argv[], char* msg) {
    char* str = get_input_string(i, argc, argv, msg);
    int num = (int) strtol(str, NULL, 10);
    if (*i >= argc) free(str);
    return num;
}

int send_signals(pid_t catcher_PID, int signals_count) {
    for (int i = 0; i < signals_count; i++) {
        printf("Sending %d signal to the CATCHER... (%d/%d sent)\n", target_signal, i + 1, signals_count);
        if (send_signal(target_signal, catcher_PID) == -1) return -1;
    }

    printf("Sending %d signal to the CATCHER...\n\n", stop_signal);
    if (send_signal(stop_signal, catcher_PID) == -1) return -1;
    sent_signals_count--; // Don't count the STOP_SIGNAL

    return 0;
}

int send_signal(int sig_no, pid_t catcher_PID) {
    if (strcmp(mode, KILL_MODE) == 0) {
        if (kill(catcher_PID, sig_no) == -1) {
            perror("Unable to send a signal.\n");
            return -1;
        }
    } else if (strcmp(mode, SIGQUEUE_MODE) == 0) {
        static union sigval value;
        if (sigqueue(catcher_PID, sig_no, value) == -1) {
            perror("Unable to add a signal to the queue.\n");
            return -1;
        }
    } else if (strcmp(mode, SIGRT_MODE) == 0) {
        if (kill(catcher_PID, sig_no) == -1) {
            perror("Unable to send a signal.\n");
            return -1;
        }
    } else {
        fprintf(stderr, "Unrecognised mode. Expected '%s', '%s' or '%s'.\n", KILL_MODE, SIGQUEUE_MODE, SIGRT_MODE);
        return -1;
    }

    sent_signals_count++;
    return 0;
}

int set_sa_sigaction(int sig_no, int sa_flags, void (*handler)(int, siginfo_t*, void*)) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_sigaction = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        perror("Action cannot be set for the signal.\n");
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

int set_up_signal_handlers(void) {
    if (set_sa_sigaction(target_signal, SA_SIGINFO, target_signal_handler) == -1 ||
        set_sa_handler(stop_signal, 0, stop_signal_handler) == -1) {
        return -1;
    }

    return 0;
}

void set_signal_numbers(void) {
    if (strcmp(mode, SIGRT_MODE) != 0) {
        target_signal = TARGET_SIGNAL;
        stop_signal = STOP_SIGNAL;
    } else {
        target_signal = SIGRT_TARGET_SIGNAL;
        stop_signal = SIGRT_STOP_SIGNAL;
    }
}

void target_signal_handler(int sig_no, siginfo_t *info, void *ucontext) {
    received_signals_count++;
    printf("Received %d from the CATCHER\n", sig_no);
    printf("Received to sent signals ratio: %d/%d\n", received_signals_count, sent_signals_count);

    if (strcmp(mode, SIGQUEUE_MODE) == 0) {
        printf("Total number of signals sent back by the CATCHER: %d\n", info->si_value.sival_int);
    }
}

void stop_signal_handler(int sig_no) {
    printf("\nReceived %d signal\n", sig_no);
}

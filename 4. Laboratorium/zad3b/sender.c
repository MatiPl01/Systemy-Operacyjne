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

static sigset_t mask;
int received_signals_count = 0;
int sent_signals_count = 0;
int signals_count;
int target_signal;
int stop_signal;
char* mode = "";

char* get_input_string(int *i, int argc, char* argv[], char* msg);
int get_input_num(int *i, int argc, char* argv[], char* msg);
int send_signal(int sig_no, pid_t catcher_PID);
int set_sa_sigaction(int sig_no, int sa_flags, void (*handler)(int, siginfo_t*, void*));
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int update_blocking_mask(void);
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

    signals_count = get_input_num(&i, argc, argv, "Please provide a number of signals to send");
    if (signals_count < 1) {
        fprintf(stderr, "A number of signals to send should be a positive integer.\n");
        return 1;
    }

    mode = get_input_string(&i, argc, argv, "Please provide a sending signals mode");

    // Set numbers of target and stop signals
    set_signal_numbers();

    // Display settings
    printf("Sender (PID: %d):\n", getpid());
    printf("\tSettings:\n");
    printf("\tCatcher's PID:     %d\n", catcher_PID);
    printf("\tNumber of signals: %d\n", signals_count);
    printf("\tMode:              %s\n\n", mode);

    // Set up signal handlers
    if (set_up_signal_handlers() == -1 ||
        // Create a blocking mask depending on the mode set
        update_blocking_mask() == -1 ||
        // Send signals to the catcher
        send_signal(target_signal, catcher_PID) == -1)
    {
        if (argc < 4) free(mode);
        return 1;
    }

    // Wait for the catcher's response
    if (sigsuspend(&mask) == -1) {
        perror("Something went wrong while calling sigsuspend.\n");
        exit(1);
    }

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

int send_signal(int sig_no, pid_t catcher_PID) {
    if (sig_no == TARGET_SIGNAL || sig_no == SIGRT_TARGET_SIGNAL) {
        printf("Sending %d signal to the CATCHER... (%d/%d sent)\n",
               target_signal, sent_signals_count, signals_count);
    } else if (sig_no == STOP_SIGNAL || sig_no == SIGRT_STOP_SIGNAL) {
        printf("Sending %d signal to the CATCHER...\n\n", stop_signal);
        sent_signals_count--; // Don't count the STOP_SIGNAL
    }
    if (strcmp(mode, KILL_MODE) == 0 || strcmp(mode, SIGRT_MODE) == 0) {
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

int update_blocking_mask(void) {
    // Initialize the mask with all signals
    if (sigfillset(&mask) == -1) {
        perror("Unable to initialize the mask with all signals.\n");
        return -1;
    }

    // Remove all accepted signals from the blocking mask
    if (strcmp(mode, KILL_MODE) == 0 || strcmp(mode, SIGQUEUE_MODE) == 0) {
        if (sigdelset(&mask, TARGET_SIGNAL) == -1 || sigdelset(&mask, STOP_SIGNAL) == -1) {
            perror("Unable to remove mode related signals from the mask.\n");
            return -1;
        }
    } else if (strcmp(mode, SIGRT_MODE) == 0) {
        if (sigdelset(&mask, SIGRT_TARGET_SIGNAL) == -1 || sigdelset(&mask, SIGRT_STOP_SIGNAL) == -1) {
            perror("Unable to remove mode related signals from the mask.\n");
            return -1;
        }
    } else {
        fprintf(stderr, "Unable to create blocking mask. Unrecognized mode.\n");
        return -1;
    }

    // Enable the blocking mask
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        perror("Unable to set the blocking mask.\n");
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

    pid_t catcher_PID = info->si_pid;
    if (sent_signals_count < signals_count) {
        send_signal(target_signal, catcher_PID);
    } else if (sent_signals_count == signals_count) {
        send_signal(stop_signal, catcher_PID);
    } else {
        fprintf(stderr, "There are no more signals to send.\n");
        exit(1);
    }

    // Wait for the catcher's response
    if (sigsuspend(&mask) == -1) {
        perror("Something went wrong while calling sigsuspend.\n");
        exit(1);
    }
}

void stop_signal_handler(int sig_no) {
    printf("\nReceived %d signal\n", sig_no);
    printf("\nI've also finished! Thank you CATCHER for good cooperation ❤.\n");
    puts("Here are my statistics:");
    printf("Total number of signals sent:     %d\n", sent_signals_count);
    printf("Total number of signals received: %d\n", received_signals_count);
    exit(0);
}

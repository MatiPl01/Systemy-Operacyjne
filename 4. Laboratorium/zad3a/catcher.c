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
char* mode = "";

char* get_input_string(int *i, int argc, char* argv[], char* msg);
int validate_mode(void);
int send_signals(pid_t sender_PID, int signals_count);
int send_signal(pid_t sender_PID, int sig_no, int sent_count);
int set_sa_sigaction(int sig_no, int sa_flags, void (*handler)(int, siginfo_t*, void*));
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int set_up_signal_handlers(void);
int set_up_unwanted_signals_blocking(void);
void target_signal_handler(int sig_no);
void stop_signal_handler(int sig_no, siginfo_t *info, void *ucontext);


int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    // Get mode from input
    int i = 1;
    mode = get_input_string(&i, argc, argv, "Please provide a sending signals mode");
    if (validate_mode() == -1) return -1;

    // Display settings
    printf("Catcher (PID: %d):\n", getpid());
    printf("\tSettings:\n");
    printf("\tMode: %s\n\n", mode);

    // Set up signal handlers
    if (set_up_signal_handlers() == -1) return 1;

    // Set up the blocking mask
    if (set_up_unwanted_signals_blocking() == -1) return 1;

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

int validate_mode(void) {
    if (strcmp(mode, KILL_MODE) == 0 || strcmp(mode, SIGQUEUE_MODE) == 0 || strcmp(mode, SIGRT_MODE) == 0) return 0;
    fprintf(stderr, "Unrecognised mode. Expected '%s', '%s' or '%s'.\n", KILL_MODE, SIGQUEUE_MODE, SIGRT_MODE);
    return -1;
}

int send_signals(pid_t sender_PID, int signals_count) {
    for (int i = 0; i < signals_count; i++) {
        printf("Sending %d signal to the SENDER... (%d/%d sent)\n", TARGET_SIGNAL, i + 1, signals_count);
        if (send_signal(sender_PID,TARGET_SIGNAL, i + 1) == -1) return -1;
    }
    printf("Sending %d signal to the SENDER...\n", STOP_SIGNAL);
    if (send_signal(sender_PID, STOP_SIGNAL, -1) == -1) return -1;
    return 0;
}

int send_signal(pid_t sender_PID, int sig_no, int sent_count) {
    if (strcmp(mode, KILL_MODE) == 0) {
        if (kill(sender_PID, sig_no) == -1) {
            perror("Unable to send a signal.\n");
            return -1;
        }
    } else if (strcmp(mode, SIGQUEUE_MODE) == 0) {
        static union sigval value;
        // Send a payload the number of signals resent back to the sender
        if (sent_count > 0) value.sival_int = sent_count;
        if (sigqueue(sender_PID, sig_no, value) == -1) {
            perror("Unable to add a signal to the queue.\n");
            return -1;
        }
    } else if (strcmp(mode, SIGRT_MODE) == 0) {
        if (kill(sender_PID, SIGRTMIN + 1) == -1) {
            perror("Unable to send a signal.\n");
            return -1;
        }
    } else {
        fprintf(stderr, "Unrecognised mode. Expected '%s', '%s' or '%s'.\n", KILL_MODE, SIGQUEUE_MODE, SIGRT_MODE);
        return -1;
    }

    return 0;
}

void target_signal_handler(int sig_no) {
    printf("Received %d from SENDER\n", sig_no);
    printf("Total number of received signals: %d\n", received_signals_count);
    received_signals_count++;
}

void stop_signal_handler(int sig_no, siginfo_t *info, void *ucontext) {
    printf("\nReceived %d signal. Total number of received signals: %d.\n\n", sig_no, received_signals_count);
    // Get sender process PID
    pid_t sender_PID = info->si_pid;
    // Send back signals to the sender process
    send_signals(sender_PID, received_signals_count);
    // Exit the catcher process
    printf("\nAll done! My job is finished 🙂\n");
    exit(0);
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
    int target_signal, stop_signal;

    if (strcmp(mode, SIGRT_MODE) != 0) {
        target_signal = TARGET_SIGNAL;
        stop_signal = STOP_SIGNAL;
    } else {
        target_signal = SIGRT_TARGET_SIGNAL;
        stop_signal = SIGRT_STOP_SIGNAL;
    }

    if (set_sa_handler(target_signal, 0, target_signal_handler) == -1 ||
        set_sa_sigaction(stop_signal, SA_SIGINFO, stop_signal_handler) == -1) {
        return -1;
    }

    return 0;
}

int set_up_unwanted_signals_blocking(void) {
    sigset_t mask;

    // Initialize the mask with all signals
    if (sigfillset(&mask) == -1) {
        perror("Unable to initialize the mask with all signals.\n");
        return -1;
    }

    // Remove only signals which the catcher should wait for
    if ((strcmp(mode, SIGRT_MODE) != 0 && (sigdelset(&mask, TARGET_SIGNAL) == -1 || sigdelset(&mask, STOP_SIGNAL) == -1))
        || (sigdelset(&mask, SIGRT_TARGET_SIGNAL) == -1 || sigdelset(&mask, SIGRT_STOP_SIGNAL) == -1)) {
        perror("Unable to remove mode related signals from the mask.\n");
        return -1;
    }

    // Enable the blocking mask
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        perror("Unable to set the blocking mask.\n");
        return -1;
    }

    // Wait for a signal
    sigsuspend(&mask);

    return 0;
}

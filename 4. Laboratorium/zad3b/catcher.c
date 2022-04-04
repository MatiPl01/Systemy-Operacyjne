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

typedef struct AcceptedSignals {
    int *signals;
    int count;
} AcceptedSignals;

int send_signal(pid_t sender_PID, int sig_no, int sent_count);
int set_sa_sigaction(int sig_no, int sa_flags, void (*handler)(int, siginfo_t*, void*));
int set_up_signal_handlers(void);
int update_mode(siginfo_t *info);
int update_accepted_signals(void);
int update_blocking_mask(void);
void set_signal_numbers(void);
void update_mode_to_first_signal(siginfo_t *info);
void target_signal_handler(int sig_no, siginfo_t *info, void *ucontext);
void stop_signal_handler(int sig_no, siginfo_t *info, void *ucontext);

static AcceptedSignals as;
static sigset_t mask;
int received_signals_count = 0;
int sent_signals_count = 0;
int target_signal;
int stop_signal;
char* mode = "";


int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    // Display settings
    printf("Catcher (PID: %d):\n", getpid());
    puts("Waiting for signals from the SENDER...\n");

    // Set up signal handlers
    if (set_up_signal_handlers() == -1) return 1;

    // Set up the accepted signals struct
    if (update_accepted_signals() == -1) return 1;

    // Set up the blocking mask
    if (update_blocking_mask() == -1) return 1;

    // Wait for the first signal from the searcher
    if (sigsuspend(&mask) == -1) {
        perror("Something went wrong while calling sigsuspend.\n");
        exit(1);
    }

    return 0;
}


int send_signal(pid_t sender_PID, int sig_no, int sent_count) {
    if (strcmp(mode, KILL_MODE) == 0 || strcmp(mode, SIGRT_MODE) == 0) {
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

int set_up_signal_handlers(void) {
    if (set_sa_sigaction(TARGET_SIGNAL, SA_SIGINFO, target_signal_handler) == -1 ||
        set_sa_sigaction(SIGRT_TARGET_SIGNAL, SA_SIGINFO, target_signal_handler) == -1 ||
        set_sa_sigaction(STOP_SIGNAL, SA_SIGINFO, stop_signal_handler) == -1 ||
        set_sa_sigaction(SIGRT_STOP_SIGNAL, SA_SIGINFO, stop_signal_handler) == -1) {
        return -1;
    }

    return 0;
}

int update_mode(siginfo_t *info) {
    int sig_no = info->si_signo;
    int sig_code = info->si_code;
    char* _mode = "";

    if (sig_code == SI_USER) {
        if (sig_no == TARGET_SIGNAL || sig_no == STOP_SIGNAL) _mode = KILL_MODE;
        if (sig_no == SIGRT_TARGET_SIGNAL || sig_no == SIGRT_STOP_SIGNAL) _mode = SIGRT_MODE;
    } else if (sig_code == SI_QUEUE) {
        if (sig_no == TARGET_SIGNAL || sig_no == STOP_SIGNAL) _mode = SIGQUEUE_MODE;
    }

    if (strlen(_mode) == 0) {
        fprintf(stderr, "Unable to update mode. Received a signal in unrecognized mode.\n");
        return -1;
    }

    // Set the global mode variable to the signal's mode
    mode = _mode;
    printf("\nSetting CATCHER mode to '%s' (to the first received signal's mode)\n\n", mode);
    set_signal_numbers();

    // Update the list of accepted signals and the blocking mask
    return (update_accepted_signals() != -1 && update_blocking_mask() != -1);
}

int update_accepted_signals(void) {
    if (strcmp(mode, "") == 0) {
        as.count = 4;
    } else if (strcmp(mode, KILL_MODE) == 0 ||
               strcmp(mode, SIGQUEUE_MODE) == 0 ||
               strcmp(mode, SIGRT_MODE) == 0) {
        as.count = 2;
    } else {
        fprintf(stderr, "Incorrect mode. Unable to create a list of allowed signals.\n");
        return -1;
    }

    if (as.signals) free(as.signals);
    as.signals = (int*) calloc(as.count, sizeof(int));
    if (!as.signals) {
        perror("Unable to allocate memory.\n");
        return -1;
    }

    if (as.count == 4) {
        as.signals[0] = TARGET_SIGNAL;
        as.signals[1] = STOP_SIGNAL;
        as.signals[2] = SIGRT_TARGET_SIGNAL;
        as.signals[3] = SIGRT_STOP_SIGNAL;
    } else {
        if (strcmp(mode, KILL_MODE) == 0 || strcmp(mode, SIGQUEUE_MODE) == 0) {
            as.signals[0] = TARGET_SIGNAL;
            as.signals[1] = STOP_SIGNAL;
        } else {
            as.signals[0] = SIGRT_TARGET_SIGNAL;
            as.signals[1] = SIGRT_STOP_SIGNAL;
        }
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
    for (int i = 0; i < as.count; i++) {
        if (sigdelset(&mask, as.signals[i]) == -1) {
            perror("Unable to remove mode related signals from the mask.\n");
            return -1;
        }
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
    printf("Received %d from the SENDER\n", sig_no);
    printf("Total number of signals received: %d\n", received_signals_count);

    if (received_signals_count == 1) {
        update_mode_to_first_signal(info);
        update_blocking_mask();
    }

    // Get sender process PID
    pid_t sender_PID = info->si_pid;
    // Send back the target signal
    if (send_signal(sender_PID, sig_no, sent_signals_count) == -1) {
        fprintf(stderr, "Something went wrong while sending %d signal to the SENDER.\n", sig_no);
        exit(1);
    }
    printf("Sending %d signal to the CATCHER... (%d/%d sent)\n", target_signal, sent_signals_count, received_signals_count);

    if (sigsuspend(&mask) == -1) {
        perror("Something went wrong while calling sigsuspend.\n");
        exit(1);
    }
}

void stop_signal_handler(int sig_no, siginfo_t *info, void *ucontext) {
    printf("\nReceived %d signal from the SENDER\n\n", sig_no);
    if (strcmp(mode, "") == 0) update_mode_to_first_signal(info);
    // Get sender process PID
    pid_t sender_PID = info->si_pid;
    // Send back the stop signal
    sent_signals_count--; // Don't count the STOP_SIGNAL
    if (send_signal(sender_PID, sig_no, sent_signals_count) == -1) {
        fprintf(stderr, "Something went wrong while sending %d signal to the SENDER.\n", sig_no);
        exit(1);
    }
    // Exit the catcher process
    printf("\nAll done! My job is finished 🙂\n");
    puts("Here are my statistics:");
    printf("Total number of signals received: %d\n", received_signals_count);
    printf("Total number of signals sent:     %d\n", sent_signals_count);
    exit(0);
}

void update_mode_to_first_signal(siginfo_t *info) {
    if (update_mode(info) == -1) {
        fprintf(stderr, "Unable to update the mode.\n");
        exit(1);
    }
}

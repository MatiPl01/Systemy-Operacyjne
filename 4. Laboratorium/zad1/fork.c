#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define IGNORE_ACTION_NAME  "ignore"
#define HANDLER_ACTION_NAME "handler"
#define MASK_ACTION_NAME    "mask"
#define PENDING_ACTION_NAME "pending"

#define USER_SIGNAL SIGUSR1

char* get_arg(int argc, char* argv[]);
char* get_input_string(char* msg);
int handle_action(char* action);
int handle_ignore_action(unsigned sig_no);
int handle_handler_action(unsigned sig_no);
int handle_mask_action(unsigned sig_no);
int handle_pending_action(unsigned sig_no);
int setup_sigaction(int sig_no, void (*handler)(int));
int setup_masked_signals(int sig_no, int* masked_signals, unsigned signals_no);
int is_signal_pending(int sig_no);
int raise_in_child_process(int sig_no);
void handler(int sig_no);
void print_centered(char* text, unsigned width, char fill_char);
void print_header(char* action);


int main(int argc, char* argv[]) {
    char* arg = get_arg(argc, argv);
    if (arg == NULL) return 1;

    if (handle_action(arg) == -1) return 1;

    return 0;
}

char* get_arg(int argc, char* argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Expected 1 program argument, got %d.\n", argc - 1);
        return NULL;
    }
    if (argc < 2) {
        return get_input_string("Please enter the action");
    }
    return argv[1];
}

char* get_input_string(char* msg) {
    printf("%s\n>>> ", msg);
    char* line;
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

int handle_action(char* action) {
    if (strcmp(action, IGNORE_ACTION_NAME) == 0) {
        return handle_ignore_action(USER_SIGNAL);
    }
    if (strcmp(action, HANDLER_ACTION_NAME) == 0) {
        return handle_handler_action(USER_SIGNAL);
    }
    if (strcmp(action, MASK_ACTION_NAME) == 0) {
        return handle_mask_action(USER_SIGNAL);
    }
    if (strcmp(action, PENDING_ACTION_NAME) == 0) {
        return handle_pending_action(USER_SIGNAL);
    }

    fprintf(stderr, "Undefined action\n");
    return -1;
}

int handle_ignore_action(unsigned sig_no) {
    print_header(IGNORE_ACTION_NAME);
    // Set the ignore action to the sig_no signal
    if (setup_sigaction(sig_no, SIG_IGN) == -1) return -1;
    // Raise the sig_no signal in a parent process
    printf("Raising a signal '%d' in a parent process.\n", sig_no);
    raise(sig_no); // same as kill(getpid(), sig_no)
    // Raise the sig_no signal in a child process
    return raise_in_child_process(sig_no);
}

int handle_handler_action(unsigned sig_no) {
    print_header(HANDLER_ACTION_NAME);
    // Set the handler action to the sig_no signal
    if (setup_sigaction(sig_no, handler) == -1) return -1;
    // Raise the sig_no signal in a parent process
    printf("Raising a signal '%d' in a parent process.\n", sig_no);
    raise(sig_no); // same as kill(getpid(), sig_no)
    // Raise the sig_no signal in a child process
    return raise_in_child_process(sig_no);
}

int handle_mask_action(unsigned sig_no) {
    print_header(MASK_ACTION_NAME);
    // Create the list of signals that should be masked
    int masked[] = { USER_SIGNAL };
    // Set up the mask
    if (setup_masked_signals(sig_no, masked, 1) == -1) return -1;
    // Raise the sig_no signal in a parent process
    printf("Raising a signal '%d' in a parent process.\n", sig_no);
    raise(sig_no); // same as kill(getpid(), sig_no)
    // Raise the sig_no signal in a child process
    return raise_in_child_process(sig_no);
}

int handle_pending_action(unsigned sig_no) {
    print_header(PENDING_ACTION_NAME);
    // Create the list of signals that should be masked
    int masked[] = { USER_SIGNAL };
    // Set up the mask
    if (setup_masked_signals(sig_no, masked, 1) == -1) return -1;
    // Raise the sig_no signal in a parent process
    printf("Raising a signal '%d' in a parent process.\n", sig_no);
    raise(sig_no); // same as kill(getpid(), sig_no)
    // Raise the sig_no signal in a child process
    if (raise_in_child_process(sig_no) == -1) return -1;
    // Check if a sig_no signal is pending
    int status = is_signal_pending(sig_no);
    switch(status) {
        case 1:
            printf("Signal %d is pending.\n", sig_no);
            break;
        case 0:
            printf("Signal %d is not pending.\n", sig_no);
            break;
        default:
            return -1;
    }

    return 0;
}

int setup_sigaction(int sig_no, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;

    if (sigaction(sig_no, &sa, NULL) == 1) {
        perror("Action cannot be set for the signal.\n");
        return -1;
    }

    return 0;
}

int setup_masked_signals(int sig_no, int* masked_signals, unsigned signals_no) {
    sigset_t mask;
    sigemptyset(&mask);
    for (unsigned i = 0; i < signals_no; i++) sigaddset(&mask, masked_signals[i]);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("Unable to set signal blocking mask.\n");
        return -1;
    }

    return 0;
}

int is_signal_pending(int sig_no) {
    sigset_t pending;

    if (sigpending(&pending) == -1) {
        perror("Pending signals cannot be examined.\n");
        return -1;
    }

    return sigismember(&pending, sig_no);
}

int raise_in_child_process(int sig_no) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("The child process could not be created.\n");
        return -1;
    }

    if (pid == 0) {
        printf("Raising a signal '%d' in a child process.\n", sig_no);
        raise(sig_no); // same as kill(getpid(), sig_no)
    } else {
        wait(NULL);
    }

    return 0;
}

void handler(int sig_no) {
    printf("Handler received signal %d. PID: %d, PPID: %d\n", sig_no, getpid(), getppid());
}

void print_centered(char* text, unsigned width, char fill_char) {
    unsigned length = strlen(text);

    if (length >= width) {
        puts(text);
        return;
    }

    unsigned l_just = (width - length) / 2;
    for (unsigned i = 0; i < l_just; i++) printf("%c", fill_char);
    printf("%s", text);
    for (unsigned i = 0; i < width - length - l_just; i++) printf("%c", fill_char);
    printf("\n");
}

void print_header(char* action) {
    char buff[41];
    sprintf(buff, " In '%s' handler ", action);
    print_centered(buff, 40, '=');
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define IGNORE_ACTION_NAME  "ignore"
#define MASK_ACTION_NAME    "mask"
#define PENDING_ACTION_NAME "pending"

#define USER_SIGNAL         SIGUSR1
#define CHILD_PROGRAM_NAME  "./child"


char* get_action(int argc, char* argv[]);
char* get_input_string(char* msg);
int handle_action(char* action);
int handle_ignore_action(int sig_no);
int handle_mask_action(int sig_no);
int handle_pending_action(int sig_no);
int setup_sigaction(int sig_no, void (*handler)(int));
int setup_masked_signals(int* masked_signals, int signals_no);
int raise_signal(int sig_no);
int exec_child(char* action, int sig_no);
void print_centered(char* text, int width, char fill_char);
void print_header(char* action);


int main(int argc, char* argv[]) {
    char* action = get_action(argc, argv);
    if (action == NULL) return 1;

    if (handle_action(action) == -1) {
        if (argc < 2) free(action);
        return 1;
    }

    // If everything succeeds, a code after the if statement above
    // won't be executed, so I skip the return statement
}

char* get_action(int argc, char* argv[]) {
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
    char* line = "";
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
    if (strcmp(action, MASK_ACTION_NAME) == 0) {
        return handle_mask_action(USER_SIGNAL);
    }
    if (strcmp(action, PENDING_ACTION_NAME) == 0) {
        return handle_pending_action(USER_SIGNAL);
    }

    fprintf(stderr, "Undefined action\n");
    return -1;
}

int handle_ignore_action(int sig_no) {
    print_header(IGNORE_ACTION_NAME);
    printf("v Parent process:\n");
    // Set the ignore action to the sig_no signal
    if (setup_sigaction(sig_no, SIG_IGN) == -1) return -1;
    // Raise the sig_no signal in a parent process
    if (raise_signal(sig_no) == -1) return -1; // alternatively use kill(getpid(), sig_no)
    // Raise the sig_no signal in a child process
    return exec_child(IGNORE_ACTION_NAME, sig_no);
}

int handle_mask_action(int sig_no) {
    print_header(MASK_ACTION_NAME);
    printf("v Parent process:\n");
    // Create the list of signals that should be masked
    int masked[] = { USER_SIGNAL };
    // Set up the mask
    if (setup_masked_signals(masked, 1) == -1) return -1;
    // Raise the sig_no signal in a parent process
    if (raise_signal(sig_no) == -1) return -1; // alternatively use kill(getpid(), sig_no)
    // Raise the sig_no signal in a child process
    return exec_child(MASK_ACTION_NAME, sig_no);
}

int handle_pending_action(int sig_no) {
    print_header(PENDING_ACTION_NAME);
    printf("v Parent process:\n");
    // Create the list of signals that should be masked
    int masked[] = { USER_SIGNAL };
    // Set up the mask
    if (setup_masked_signals(masked, 1) == -1) return -1;
    // Raise the sig_no signal in a parent process
    if (raise_signal(sig_no) == -1) return -1; // alternatively use kill(getpid(), sig_no)
    // Check if a signal is pending in a child process
    return exec_child(PENDING_ACTION_NAME, sig_no);
}

int setup_sigaction(int sig_no, void (*handler)(int)) {
    struct sigaction *sa = (struct sigaction*) calloc(1, sizeof(struct sigaction));
    sa->sa_handler = handler;
    sa->sa_flags = SA_RESTART;

    if (sigaction(sig_no, sa, NULL) == 1) {
        perror("Action cannot be set for the signal.\n");
        return -1;
    }

    return 0;
}

int setup_masked_signals(int* masked_signals, int signals_no) {
    sigset_t mask;
    sigemptyset(&mask);
    for (int i = 0; i < signals_no; i++) sigaddset(&mask, masked_signals[i]);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("Unable to set signal blocking mask.\n");
        return -1;
    }

    return 0;
}

int raise_signal(int sig_no) {
    printf("Raising a signal '%d'.\n", sig_no);

    if (raise((int) sig_no) != 0) {
        perror("Unable to raise a signal.\n");
        return -1;
    }

    return 0;
}

int exec_child(char* action, int sig_no) {
    char buff[32];
    sprintf(buff, "%d", sig_no);

    // Flush the parent's output to the stdout to
    // prevent it from getting overwritten by the
    // child's process output
    fflush(stdout);

    if (execl(CHILD_PROGRAM_NAME, CHILD_PROGRAM_NAME, action, buff, NULL) == -1) {
        perror("Something went wrong while executing a child process.\n");
        return -1;
    }

    return 0;
}

void print_centered(char* text, int width, char fill_char) {
    size_t length = strlen(text);

    if (length >= width) {
        puts(text);
        return;
    }

    size_t l_just = (width - length) / 2;
    for (int i = 0; i < l_just; i++) printf("%c", fill_char);
    printf("%s", text);
    for (int i = 0; i < width - length - l_just; i++) printf("%c", fill_char);
    printf("\n");
}

void print_header(char* action) {
    char buff[41];
    sprintf(buff, " Action: '%s' ", action);
    print_centered(buff, 40, '=');
}

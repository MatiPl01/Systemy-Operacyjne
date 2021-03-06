#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

#define SIGINFO_TEST_NAME "SIGINFO"
#define NOCLDSTOP_TEST_NAME "NOCLDSTOP"
#define RESETHAND_TEST_NAME "RESETHAND"

int send_signal(int pid, int sig_no);
int run_test(char* test_name);
int test_SA_SIGINFO(void);
int test_SA_SIGINFO_child_fn(void);
int test_SA_SIGINFO_parent_fn(int child_pid);
int test_SA_NOCLDSTOP(void);
int test_SA_NOCLDSTOP_child_fn(void);
int test_SA_NOCLDSTOP_parent_fn_1(int child_pid);
int test_SA_NOCLDSTOP_parent_fn_2(int child_pid);
int test_SA_RESETHAND(void);
int test_SA_RESETHAND_child_fn(void);
int test_SA_RESETHAND_parent_fn_1(int child_pid);
int test_SA_RESETHAND_parent_fn_2(int child_pid);
int set_sa_sigaction(int sig_no, int sa_flags, void (*handler)(int, siginfo_t*, void*));
int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int));
int fork_and_execute(int (*child_fn)(void), int (*parent_fn)(int));
void handle_child_SIGINT_SIGINFO(int sig_no, siginfo_t *info, void *ucontext);
void handle_parent_SIGINT_SIGINFO(int sig_no, siginfo_t *info, void *ucontext);
void handle_parent_SIGCHLD_SIGINFO(int sig_no, siginfo_t *info, void *ucontext);
void handle_SIGCHLD_NOCLDSTOP(int sig_no);
void handle_SIGCHLD_RESETHAND(int sig_no);
void print_centered(char* text, int width, char fill_char);
void print_info(int sig_no, siginfo_t *info);


int main(int argc, char* argv[]) {
    // Run all tests if no test name was specified
    if (argc == 1) {
        if (test_SA_NOCLDSTOP() == -1) return 1;
        if (test_SA_RESETHAND() == -1) return 1;
        // SIGINFO test must be called last as a parent will exit after receiving SIGCHLD signal
        if (test_SA_SIGINFO() == -1) return 1;
        // Otherwise, run only the specified tests
    } else {
        for (int i = 0; i < argc; i++) {
            if (run_test(argv[i]) == -1) return 1;
        }
    }

    return 0;
}


int send_signal(int pid, int sig_no) {
    if (kill(pid, sig_no) == -1) {
        perror("Unable to send signal.\n");
        return -1;
    }
    return 0;
}

int run_test(char* test_name) {
    if (strcmp(test_name, SIGINFO_TEST_NAME) == 0) {
        return test_SA_SIGINFO();
    }
    if (strcmp(test_name, NOCLDSTOP_TEST_NAME) == 0) {
        return test_SA_NOCLDSTOP();
    }
    if (strcmp(test_name, RESETHAND_TEST_NAME) == 0) {
        return test_SA_RESETHAND();
    }

    return 0;
}

/*
 * SIGINFO TESTS
 */
int test_SA_SIGINFO(void) {
    printf("\n");
    print_centered(" SIGINFO test ", 40, '=');
    // Set sigaction in the parent process
    if (set_sa_sigaction(SIGINT, SA_SIGINFO, handle_parent_SIGINT_SIGINFO) == -1) return -1;
    // Set sigaction in the child process
    if (set_sa_sigaction(SIGCHLD, SA_SIGINFO, handle_parent_SIGCHLD_SIGINFO) == -1) return -1;

    return fork_and_execute(test_SA_SIGINFO_child_fn, test_SA_SIGINFO_parent_fn);
}

int test_SA_SIGINFO_child_fn(void) {
    // Set sigaction in the child process
    if (set_sa_sigaction(SIGINT, 0, handle_child_SIGINT_SIGINFO) == -1) exit(1);
    // Send the SIGINT signal to the parent process
    if (send_signal(getppid(), SIGINT) == -1) return -1;
    // Pause the child process
    pause(); // This will wait for a signal of any type
    return 0;
}

int test_SA_SIGINFO_parent_fn(int pid) {
    // Create an infinite loop in the parent process
    while (true) {}
}

void handle_parent_SIGINT_SIGINFO(int sig_no, siginfo_t *info, void *ucontext) {
    puts("\nHandling SIGINT in a parent");
    print_info(sig_no, info);
    send_signal(info->si_pid, SIGINT);
}

void handle_parent_SIGCHLD_SIGINFO(int sig_no, siginfo_t *info, void *ucontext) {
    puts("\nHandling SIGCHLD in a parent");
    print_info(sig_no, info);
    exit(0);
}

void handle_child_SIGINT_SIGINFO(int sig_no, siginfo_t *info, void *ucontext) {
    // When siginfo is called in a handler for SIGINT, it returns NULL.
    // https://support.sas.com/documentation/onlinedoc/ccompiler/doc700/html/lr1/zid-3640.htm
    puts("\nHandling SIGINT in a child");
    printf("SIG %d. PID: %d, PPID: %d.\n", sig_no, getpid(), getppid());
    printf("No additional information. Is siginfo NULL? %s\n", info ? "no" : "yes");
    exit(0);
}

/*
 * NOCLDSTOP TESTS
 */
int test_SA_NOCLDSTOP(void) {
    // If signum is SIGCHLD, do not receive notification when
    // child processes stop (i.e., when they receive one of
    // SIGSTOP, SIGTSTP, SIGTTIN, or SIGTTOU) or resume (i.e.,
    // they receive SIGCONT) This flag is meaningful only when
    // establishing a handler for SIGCHLD.
    print_centered(" NOCLDSTOP test ", 40, '=');

    /*
     * WITHOUT NOCLDSTOP FLAG SET
     */
    // Set sigaction in the parent process
    if (set_sa_handler(SIGCHLD, 0, handle_SIGCHLD_NOCLDSTOP) == -1) return -1;
    fork_and_execute(test_SA_NOCLDSTOP_child_fn, test_SA_NOCLDSTOP_parent_fn_1);

    /*
     * WITH NOCLDSTOP FLAG SET
     */
    if (set_sa_handler(SIGCHLD, SA_NOCLDSTOP, handle_SIGCHLD_NOCLDSTOP) == -1) return -1;
    fork_and_execute(test_SA_NOCLDSTOP_child_fn, test_SA_NOCLDSTOP_parent_fn_2);

    return 0;
}

int test_SA_NOCLDSTOP_child_fn(void) {
    printf("Starting and infinite loop a child process with PID %d.\n", getpid());
    while (true) {}
}

int test_SA_NOCLDSTOP_parent_fn_1(int child_pid) {
    printf("\nSending SIGSTOP to the child process with PID %d (PPID: %d).\n", child_pid, getpid());
    puts("Parent process should receive SIGCHLD");
    if (send_signal(child_pid, SIGSTOP) == -1) return -1;
    wait(NULL);
    return 0;
}

int test_SA_NOCLDSTOP_parent_fn_2(int child_pid) {
    printf("\nSending SIGSTOP to the child process with PID %d (PPID: %d).\n", child_pid, getpid());
    puts("Parent process should NOT receive SIGCHLD");
    if (send_signal(child_pid, SIGSTOP) == -1) return -1;
    return 0;
}

void handle_SIGCHLD_NOCLDSTOP(int sig_no) {
    puts("\nHandling SIGCHLD");
    printf("SIG: %d, PID: %d, PPID: %d\n", sig_no, getpid(), getppid());
}

/*
 * RESETHAND TESTS
 */
int test_SA_RESETHAND(void) {
    // Restore the signal action to the default upon entry to the
    // signal handler. This flag is meaningful only when
    // establishing a signal handler.
    // (So we expect a handler to be called only once and then no more)
    printf("\n");
    print_centered(" RESETHAND test ", 40, '=');

    if (set_sa_handler(SIGCHLD, SA_RESETHAND, handle_SIGCHLD_RESETHAND) == -1) return -1;

    // A handler should be called
    fork_and_execute(test_SA_RESETHAND_child_fn, test_SA_RESETHAND_parent_fn_2);

    // A handler shouldn't be called after executing a call from above
    for (int i = 0; i < 3; i++) {
        fork_and_execute(test_SA_RESETHAND_child_fn, test_SA_RESETHAND_parent_fn_1);
    }

    return 0;
}

int test_SA_RESETHAND_child_fn(void) {
    printf("Starting infinite loop in a child process with PID %d.\n", getpid());
    while (true) {}
}

int test_SA_RESETHAND_parent_fn_1(int child_pid) {
    printf("\nSending SIGKILL to the child process with PID %d (PPID: %d).\n", child_pid, getpid());
    puts("Parent process should IGNORE SIGCHLD");
    if (send_signal(child_pid, SIGKILL) == -1) return -1;
    wait(NULL);
    return 0;
}

int test_SA_RESETHAND_parent_fn_2(int child_pid) {
    printf("\nSending SIGKILL to the child process with PID %d (PPID: %d).\n", child_pid, getpid());
    puts("Parent process should HANDLE SIGCHLD");
    if (send_signal(child_pid, SIGKILL) == -1) return -1;
    wait(NULL);
    return 0;
}

void handle_SIGCHLD_RESETHAND(int sig_no) {
    puts("\nHandling SIGCHLD");
    printf("SIG: %d, PID: %d, PPID: %d\n", sig_no, getpid(), getppid());
}

/*
 * HELPER FUNCTIONS
 */
int fork_and_execute(int (*child_fn)(void), int (*parent_fn)(int)) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("The child process could not be created.\n");
        return -1;
    }

    fflush(stdout);
    if (pid == 0) {
        if (child_fn() == -1) {
            fprintf(stderr, "Something went wrong while executing a child's process function.\n");
            return -1;
        }
    } else {
        if (parent_fn(pid) == -1) {
            fprintf(stderr, "Something went wrong while executing a parent's process function.\n");
            return -1;
        }
    }

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

void print_info(int sig_no, siginfo_t *info) {
    printf("SIG %d. PID: %d, PPID: %d.\n", sig_no, getpid(), getppid());
    printf("Signal number: %d\n", info->si_signo);
    printf("Sender PID:    %d\n", info->si_pid);
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
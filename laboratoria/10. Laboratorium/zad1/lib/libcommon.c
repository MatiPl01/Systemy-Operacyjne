#include "libcommon.h"


char* get_input_string(int *i, int argc, char* argv[], char* msg) {
    // If an argument was provided, return this argument
    if (*i < argc) {
        char* arg = malloc(strlen(argv[*i]) + 1);
        if (!arg) {
            cperror("Unable to allocate memory for the input\n");
            return NULL;
        }
        strcpy(arg, argv[(*i)++]);
        return arg;
    }

    // Otherwise, get an input from a user
    printf("%s\n%s>>> %s", msg, C_BRIGHT_YELLOW, C_RESET);
    char* line = "";
    size_t length = 0;
    getline(&line, &length, stdin);
    // Remove the newline character
    line[strlen(line) - 1] = '\0';
    return line;
}

int get_input_num(int *i, int argc, char* argv[], char* msg) {
    char* str = get_input_string(i, argc, argv, msg);
    if (!str) return -1;
    int num = (int) strtol(str, NULL, 10);
    free(str);
    return num;
}

int set_sa_handler(int sig_no, int sa_flags, void (*handler)(int)) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_handler = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        cperror("Cannot set a signal handler for %d signal\n", sig_no);
        return -1;
    }

    return 0;
}

void free_args(int arg_count, ...) {
    va_list args_list;
    va_start(args_list, arg_count);

    for (int i = 0; i < arg_count; i++) {
        void* arg = va_arg(args_list, void*);
        if (arg) free(arg);
    }

    va_end(args_list);
}


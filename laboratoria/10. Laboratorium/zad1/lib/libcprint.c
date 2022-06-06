#include "libcprint.h"


void print_in_color(FILE *fd, char* color, char* format, va_list args) {
    char color_format[strlen(color) + strlen(C_RESET) + strlen(format) + 1];
    sprintf(color_format, "%s%s%s", color, format, C_RESET);
    vfprintf(fd, color_format, args);
    va_end(args);
}

void cfprintf(FILE *f_ptr, char* color, char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(f_ptr, color, format, args);
}

void cprintf(char* color, char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stdout, color, format, args);
}

void cperror(char* format, ...) {
    va_list args;
    va_start(args, format);
    char* err_msg = strerror(errno);
    char error_format[strlen(format) + strlen(err_msg) + 2];
    sprintf(error_format, "%s: %s", err_msg, format);
    print_in_color(stderr, C_RED, error_format, args);
}

void cerror(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stderr, C_RED, format, args);
}

void ccritical(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stdout, C_BRIGHT_RED, format, args);
}

void cwarn(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stderr, C_YELLOW, format, args);
}

void cinfo(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stderr, C_CYAN, format, args);
}

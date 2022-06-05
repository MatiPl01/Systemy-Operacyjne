#include "libprint.h"


static void print_in_color(FILE *fd, color color, char* format, va_list args) {
    char color_format[1024];
    sprintf(color_format, "%s%s%s", COLORS[color], format, COLORS[RESET]);
    vfprintf(fd, color_format, args);
    va_end(args);
}

void cfprintf(FILE *fd, color color, char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(fd, color, format, args);
}

void cprintf(color color, char* format, ...) {
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
    print_in_color(stderr, RED, error_format, args);
}

void cerror(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stderr, RED, format, args);
}

void cwarn(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stderr, YELLOW, format, args);
}

void cinfo(char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(stderr, CYAN, format, args);
}

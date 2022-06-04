#include "libprint.h"


static void print_in_color(FILE *fd, Color color, char* format, va_list args) {
    char *c = format;
    char msg[MAX_MSG_LENGTH];
    sprintf(msg, "%s", COLORS[color]);

    while (*c != '\0') {
        char temp[MAX_MSG_LENGTH];
        if (*c != '%') {
            sprintf(temp, "%c", *c);
        } else if (*++c != '\0') {
            switch (*c) {
                case 'c':
                    sprintf(temp, "%c", va_arg(args, int));
                    break;
                case 'd':
                    sprintf(temp, "%d", va_arg(args, int));
                    break;
                case 's':
                    sprintf(temp, "%s", va_arg(args, char*));
                    break;
            }
        }
        strcat(msg, temp);
        c++;
    }

    fprintf(fd, "%s%s", msg, COLORS[RESET]);
    va_end(args);
}

void cfprintf(FILE *fd, Color color, char* format, ...) {
    va_list args;
    va_start(args, format);
    print_in_color(fd, color, format, args);
}

void cprintf(Color color, char* format, ...) {
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

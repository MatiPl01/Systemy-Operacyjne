#include "libprint.h"


void printcf(Color color, char* format,...) {
    char *c = format;
    va_list args;
    va_start(args, format);
    size_t args_length = strlen(format);
    char msg[strlen(format) + strlen(COLORS[RESET]) + strlen(COLORS[color]) + 1];
    sprintf(msg, "%s", COLORS[color]);

    while (*c != '\0') {
        char temp[args_length + 1];
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

    printf("%s%s", msg, COLORS[RESET]);

    va_end(args);
}

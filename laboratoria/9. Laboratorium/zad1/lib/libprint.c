#include "libprint.h"


void printcf(Color color, char* format,...) {
    char *c = format;
    va_list lst;
    va_start(lst, format);
    char msg[1024];  // TODO - maybe change this constant to something else
    sprintf(msg, "%s", COLORS[color]);

    while (*c != '\0') {
        char temp[1024];  // TODO - maybe change this constant to something else
        if (*c != '%') {
            sprintf(temp, "%c", *c);
        } else if (*++c != '\0') {
            switch (*c) {
                case 'c':
                    sprintf(temp, "%c", va_arg(lst, int));
                    break;
                case 'd':
                    sprintf(temp, "%d", va_arg(lst, int));
                    break;
                case 's':
                    sprintf(temp, "%s", va_arg(lst, char*));
                    break;
            }
        }
        strcat(msg, temp);
        c++;
    }

    printf("%s%s", msg, COLORS[RESET]);

    va_end(lst);
}

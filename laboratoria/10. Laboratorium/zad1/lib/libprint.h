#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_MSG_LENGTH 1024

static const char* const COLORS[] = {
    "\x1B[0m",  // RESET
    "\x1B[36m", // CYAN
    "\x1B[35m", // MAGENTA
    "\x1B[33m", // YELLOW
    "\x1B[31m"  // RED
};

typedef enum Color {
    RESET,
    CYAN,
    MAGENTA,
    YELLOW,
    RED
} Color;

void cfprintf(FILE *fd, Color color, char* format, ...);
void cprintf(Color color, char* format, ...);
void cperror(char* format, ...);
void cerror(char* format, ...);

#endif // PRINT_H

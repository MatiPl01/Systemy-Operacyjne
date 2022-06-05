#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static const char* const COLORS[] = {
    "\x1B[0m",  // RESET
    "\x1B[31m", // RED
    "\x1b[32m", // GREEN
    "\x1B[33m", // YELLOW
    "\x1b[34m", // BLUE
    "\x1B[35m", // MAGENTA
    "\x1B[36m", // CYAN
    "\x1b[91m", // BRIGHT RED
    "\x1b[92m", // BRIGHT GREEN
    "\x1b[93m", // BRIGHT YELLOW
    "\x1b[94m", // BRIGHT BLUE
    "\x1b[95m", // BRIGHT MAGENTA
    "\x1b[96m", // BRIGHT CYAN
};

typedef enum color {
    RESET,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN
} color;

void cfprintf(FILE *fd, color color, char* format, ...);
void cprintf(color color, char* format, ...);
void cperror(char* format, ...);
void cerror(char* format, ...);
void cwarn(char* format, ...);
void cinfo(char* format, ...);

#endif // PRINT_H

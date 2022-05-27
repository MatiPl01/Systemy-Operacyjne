#ifndef LIBPRINT_H
#define LIBPRINT_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static const char* const COLORS[5] = {
    "\x1B[0m",
    "\x1B[36m",
    "\x1B[35m",
    "\x1B[33m"
};

typedef enum Color {
    RESET,
    CYAN,
    MAGENTA,
    YELLOW
} Color;

void printcf(Color color, char* format, ...);

#endif // LIBPRINT_H

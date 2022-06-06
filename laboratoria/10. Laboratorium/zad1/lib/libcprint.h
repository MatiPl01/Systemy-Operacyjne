#ifndef CFPRINT_H
#define CFPRINT_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define C_RESET "\x1B[0m"
#define C_RED "\x1B[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1B[33m"
#define C_BLUE "\x1b[34m"
#define C_MAGENTA "\x1B[35m"
#define C_CYAN "\x1B[36m"
#define C_BRIGHT_RED "\x1b[91m"
#define C_BRIGHT_GREEN "\x1b[92m"
#define C_BRIGHT_YELLOW "\x1b[93m"
#define C_BRIGHT_BLUE "\x1b[94m"
#define C_BRIGHT_MAGENTA "\x1b[95m"
#define C_BRIGHT_CYAN "\x1b[96m"

#define C_ERROR C_RED
#define C_WARN C_YELLOW
#define C_INFO C_CYAN

void print_in_color(FILE *f_ptr, char* color, char* format, va_list args);
void cfprintf(FILE *f_ptr, char* color, char* format, ...);
void csprintf(char* buff, char* color, char* format, ...);
void cprintf(char* color, char* format, ...);
void cperror(char* format, ...);
void cerror(char* format, ...);
void ccritical(char* format, ...);
void cwarn(char* format, ...);
void cinfo(char* format, ...);

#endif // CFPRINT_H

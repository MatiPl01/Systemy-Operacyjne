	# Used compiler
CC=gcc
# Included directories path
INCDIRS=.
# Compiler options
COPT=-O0
# Compiler flags
CFLAGS=-Wall -Wextra -Werror -std=gnu11 -g -I$(INCDIRS) $(COPT)

# Library name
LIBNAME=mylib

# Targets names
TARGETS=static shared


all: $(TARGETS)

static:
	$(CC) $(CFLAGS) -c $(LIBNAME).c
	ar rcs $(LIBNAME).a $(LIBNAME).o

shared:
	$(CC) $(CFLAGS) -fPIC -c $(LIBNAME).c
	$(CC) $(CFLAGS) -fPIC -shared $(LIBNAME).o -o $(LIBNAME).so

clean:
	rm -f *.o

clean_all:
	rm -f *.o *.a *.so

# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g $(C_OPT)

# Library name
LIB_NAME=libcopylib

# Targets names
TARGETS=static


all: $(TARGETS)

static:
	$(CC) $(C_FLAGS) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

clean:
	rm -f *.o

clean_all:
	rm -f *.o *.a

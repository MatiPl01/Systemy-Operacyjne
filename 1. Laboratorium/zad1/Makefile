# Used compiler
CC=gcc
# Included directories path
INC_DIRS=.
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -I$(INC_DIRS) $(C_OPT)

# Library name
LIB_NAME=libsysops

# Targets names
TARGETS=static shared


all: $(TARGETS)

static:
	$(CC) $(C_FLAGS) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) $(C_FLAGS) -fPIC -c $(LIB_NAME).c
	$(CC) $(C_FLAGS) -fPIC -shared $(LIB_NAME).o -o $(LIB_NAME).so

clean:
	rm -f *.o

clean_all:
	rm -f *.o *.a *.so

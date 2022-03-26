# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Library name
LIB_NAME=proc

# Targets names
TARGETS=$(LIB_NAME)_static


all: $(TARGETS)

$(LIB_NAME)_static:
	$(CC) $(C_FLAGS) -c lib$(LIB_NAME).c
	ar rcs lib$(LIB_NAME).a lib$(LIB_NAME).o

clean:
	rm -f *.o

clean_all: clean
	rm -f *.a

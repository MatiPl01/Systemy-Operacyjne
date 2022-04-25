
# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g $(C_OPT)

# Library name
LIB_NAME=libshared

# Targets names
TARGETS=shared


all: $(TARGETS)

shared:
	@$(CC) $(C_FLAGS) -fPIC -c $(LIB_NAME).c
	@$(CC) $(C_FLAGS) -fPIC -shared $(LIB_NAME).o -o $(LIB_NAME).so

clean:
	@rm -f *.o

clean_all:
	@rm -f *.o *.so

# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g $(C_OPT)

# Libraries names
COUNT_LIB_NAME=countlib
COUNT_SYS_NAME=countsys

# Targets names
TARGETS=$(COUNT_LIB_NAME)_static $(COUNT_SYS_NAME)_static


all: $(TARGETS)

$(COUNT_LIB_NAME)_static:
	@$(CC) $(C_FLAGS) -c lib$(COUNT_LIB_NAME).c
	@ar rcs lib$(COUNT_LIB_NAME).a lib$(COUNT_LIB_NAME).o

$(COUNT_SYS_NAME)_static:
	@$(CC) $(C_FLAGS) -c lib$(COUNT_SYS_NAME).c
	@ar rcs lib$(COUNT_SYS_NAME).a lib$(COUNT_SYS_NAME).o

clean:
	@rm -f *.o

clean_all:
	@rm -f *.o *.a

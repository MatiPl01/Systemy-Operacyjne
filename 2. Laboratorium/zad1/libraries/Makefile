# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g $(C_OPT)

# Libraries names
COPY_LIB_NAME=copylib
COPY_SYS_NAME=copysys

# Targets names
TARGETS=$(COPY_LIB_NAME)_static $(COPY_SYS_NAME)_static


all: $(TARGETS)

$(COPY_LIB_NAME)_static:
	$(CC) $(C_FLAGS) -c lib$(COPY_LIB_NAME).c
	ar rcs lib$(COPY_LIB_NAME).a lib$(COPY_LIB_NAME).o

$(COPY_SYS_NAME)_static:
	$(CC) $(C_FLAGS) -c lib$(COPY_SYS_NAME).c
	ar rcs lib$(COPY_SYS_NAME).a lib$(COPY_SYS_NAME).o

clean:
	rm -f *.o

clean_all:
	rm -f *.o *.a

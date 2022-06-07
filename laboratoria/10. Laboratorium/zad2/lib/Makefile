# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Libraries names
LIB_NAMES=common cprint

# Targets names
TARGETS=shared


all: $(TARGETS)

shared:
	@for LIB_NAME in $(LIB_NAMES); do \
		$(CC) $(C_FLAGS) -fPIC -c lib$$LIB_NAME.c; \
        $(CC) $(C_FLAGS) -fPIC -shared lib$$LIB_NAME.o -o lib$$LIB_NAME.so; \
	done; \

clean:
	@rm -f *.o

clean_all:
	@rm -f *.o *.so

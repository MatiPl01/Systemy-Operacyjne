# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Linked libraries
LIBRARIES=-lcommon -lcprint -lpthread
# Lib directory path
LIB_DIR=./lib
# File names
FILE_NAMES=client server

# Targets names
TARGETS=$(FILE_NAMES)


all: compile

shared_libs:
	@make -C $(LIB_DIR) shared

.PHONY: compile
compile: shared_libs
	@for FILE_NAME in $(FILE_NAMES); do \
		$(CC) $(C_FLAGS) $$FILE_NAME.c -Wl,-R $(LIB_DIR) -L$(LIB_DIR) $(LIBRARIES) -o $$FILE_NAME; \
	done; \

clean:
	@rm -f $(FILE_NAMES)
	@make -C $(LIB_DIR) clean

clean_all: clean
	@make -C $(LIB_DIR) clean_all

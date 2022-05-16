# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0

#Library directory
LIB_DIR=./lib
#Library name
LIB_NAME=common

# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -L $(LIB_DIR) $(C_OPT)

# File names
FILE_NAMES=chef deliverer main

# Targets names
TARGETS=$(FILE_NAMES)


all: compile

shared_lib:
	@make -C $(LIB_DIR) shared

.PHONY: compile
compile: shared_lib
	@COMP_NAME=''; \
	for OUT_NAME in $(FILE_NAMES); do \
		COMP_NAME=`expr $$OUT_NAME`.c; \
		$(CC) $(C_FLAGS) $$COMP_NAME -Wl,-R $(LIB_DIR) -l $(LIB_NAME) -o $$OUT_NAME; \
	done; \

clean:
	@rm -f $(FILE_NAMES)
	@make -C $(LIB_DIR) clean

clean_all: clean
	@make -C $(LIB_DIR) clean_all

# Used compiler
CC=gcc
# Library directory
LIB_DIR=./library
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g -L $(LIB_DIR) $(C_OPT)

# Compiled file name
COMP_FILE_NAME=main.c
# Output file name
OUT_FILE=main
# Library file name
LIB_NAME=proc

# Targets names
TARGETS=$(OUT_FILE)


all: clean_all $(TARGETS)

static:
	@make -C $(LIB_DIR) $(LIB_NAME)_static

$(OUT_FILE):
	@make static LIB_NAME=$(LIB_NAME)
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(LIB_NAME) -o $(OUT_FILE)

clean:
	@rm -f $(OUT_FILE)

clean_all: clean
	@make -C $(LIB_DIR) clean_all

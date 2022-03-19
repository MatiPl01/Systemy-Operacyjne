# Used compiler
CC=gcc
# Libraries directories
LIB_DIR=./libraries
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -L $(LIB_DIR) $(C_OPT)

# Compiled file name
COMP_FILE_NAME=main.c
# Libraries names
LIB_NAME=copylib
# Output file name
OUT_NAME=copy

# Targets names
TARGETS=$(OUT_NAME)


all: $(TARGETS)

static:
	make -C $(LIB_DIR) static

$(OUT_NAME): static
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(LIB_NAME) -o $(OUT_NAME)

clean:
	rm -f $(OUT_NAME)

clean_all: clean
	rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a

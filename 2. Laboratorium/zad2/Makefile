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

# Output files names
COUNT_LIB_NAME=countlib
COUNT_SYS_NAME=countsys

# Targets names
TARGETS=$(COUNT_LIB_NAME) $(COUNT_SYS_NAME)


all: $(TARGETS)

static:
	make -C $(LIB_DIR) $(LIB_NAME)_static


$(COUNT_LIB_NAME): clean
	make static LIB_NAME=$(COUNT_LIB_NAME)
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COUNT_LIB_NAME) -o $(COUNT_LIB_NAME)

$(COUNT_SYS_NAME): clean
	make static LIB_NAME=$(COUNT_SYS_NAME)
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COUNT_SYS_NAME) -o $(COUNT_SYS_NAME) -D SYS_MODE

clean:
	rm -f $(COUNT_LIB_NAME)
	rm -f $(COUNT_SYS_NAME)

clean_all: clean
	rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a

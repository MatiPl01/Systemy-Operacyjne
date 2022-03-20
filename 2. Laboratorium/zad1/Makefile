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
COPY_LIB_NAME=copylib
COPY_SYS_NAME=copysys

# Targets names
TARGETS=$(COPY_LIB_NAME) $(COPY_SYS_NAME)


all: $(TARGETS)

static:
	make -C $(LIB_DIR) $(LIB_NAME)_static

$(COPY_LIB_NAME): clean
	make static LIB_NAME=$(COPY_LIB_NAME)
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COPY_LIB_NAME) -o $(COPY_LIB_NAME)

$(COPY_SYS_NAME): clean
	make static LIB_NAME=$(COPY_SYS_NAME)
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COPY_SYS_NAME) -o $(COPY_SYS_NAME) -D LIB_SYS

clean:
	rm -f $(COPY_LIB_NAME)
	rm -f $(COPY_SYS_NAME)

clean_all: clean
	rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a

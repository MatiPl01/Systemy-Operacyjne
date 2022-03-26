# Used compiler
CC=gcc
# Libraries directories
LIB_DIR=./libraries
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g -L $(LIB_DIR) $(C_OPT)

# Compiled file name
COMP_FILE_NAME=main.c

# Output files names
LIST_DIR_STAT_NAME=listdirstat
LIST_DIR_NFTW_NAME=listdirnftw

# Targets names
TARGETS=$(LIST_DIR_STAT_NAME) $(LIST_DIR_NFTW_NAME)


all: clean $(TARGETS)

static:
	@make -C $(LIB_DIR) $(LIB_NAME)_static

$(LIST_DIR_STAT_NAME):
	@rm -f $(LIST_DIR_STAT_NAME)
	@make static LIB_NAME=$(LIST_DIR_STAT_NAME)
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(LIST_DIR_STAT_NAME) -o $(LIST_DIR_STAT_NAME)

$(LIST_DIR_NFTW_NAME):
	@rm -f $(LIST_DIR_NFTW_NAME)
	@make static LIB_NAME=$(LIST_DIR_NFTW_NAME)
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(LIST_DIR_NFTW_NAME) -o $(LIST_DIR_NFTW_NAME) -D LIB_NFTW

clean:
	@rm -f $(LIST_DIR_STAT_NAME) $(LIST_DIR_NFTW_NAME)

clean_all: clean
	@make -C $(LIB_DIR) clean_all

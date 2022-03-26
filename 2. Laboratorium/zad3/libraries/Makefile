# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Libraries names
LIST_DIR_STAT_NAME=listdirstat
LIST_DIR_NFTW_NAME=listdirnftw

# Targets names
TARGETS=$(LIST_DIR_STAT_NAME)_static $(LIST_DIR_NFTW_NAME)_static


all: $(TARGETS)

$(LIST_DIR_STAT_NAME)_static:
	@$(CC) $(C_FLAGS) -c lib$(LIST_DIR_STAT_NAME).c
	@ar rcs lib$(LIST_DIR_STAT_NAME).a lib$(LIST_DIR_STAT_NAME).o

$(LIST_DIR_NFTW_NAME)_static:
	@$(CC) $(C_FLAGS) -c lib$(LIST_DIR_NFTW_NAME).c
	@ar rcs lib$(LIST_DIR_NFTW_NAME).a lib$(LIST_DIR_NFTW_NAME).o

clean:
	@rm -f *.o

clean_all:
	@rm -f *.o *.a

# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Linked libraries
LIBRARIES=-lcommon -lprint -lrandom -lpthread
# Names of characters
CHARACTER_NAMES=elf reindeer santa
# Lib directory path
LIB_DIR=./lib
# Characters directory path
CHARACTERS_DIR=./characters
# Main file name
MAIN_FILE_NAME=main

all: compile

.PHONY: compile
compile: make_libs
	@CHARACTERS=; \
	for CHARACTER_NAME in $(CHARACTER_NAMES); do \
  		CHARACTERS=$$CHARACTERS' '$(CHARACTERS_DIR)/$$CHARACTER_NAME.c; \
  	done; \
  	$(CC) $(C_FLAGS) $(MAIN_FILE_NAME).c $$CHARACTERS -Wl,-R$(LIB_DIR) -L$(LIB_DIR) $(LIBRARIES) -o $(MAIN_FILE_NAME)

make_libs:
	@make -C $(LIB_DIR) shared

clean:
	@rm -f $(MAIN_FILE_NAME)
	@make -C $(LIB_DIR) clean
	@for CHARACTER_NAME in $(CHARACTER_NAMES); do \
		rm -rf $(CHARACTERS_DIR)/$$CHARACTER_NAME; \
	done; \

clean_all: clean
	@make -C $(LIB_DIR) clean_all

# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)
# Linker libraries flags
C_LIB_FLAGS=-lm -lpthread

# File name
FILE_NAME=main
# Time measurements file name
TIMES_FILE_NAME=times.txt
# Results directory path
RESULTS_DIR=./results

# Source files directory
SOURCES_DIR=./sources
# Test threads numbers
THREADS_NUMBERS=1 2 4 8 16 32 64 128
# Test methods names
TEST_METHODS=numbers block
# Source files names
SOURCE_FILES=dragon.ascii.pgm saturn.ascii.pgm surf.ascii.pgm


all: compile

.PHONY: compile
compile:
	$(CC) $(C_FLAGS) $(FILE_NAME).c -o $(FILE_NAME) $(C_LIB_FLAGS)

test: clean_all compile
	@for IMAGE_NAME in $(SOURCE_FILES); do \
  		echo '=================================='; \
  		echo 'Testing image: '$$IMAGE_NAME; \
  		echo '=================================='; \
  		for TEST_METHOD in $(TEST_METHODS); do \
			for NO_THREADS in $(THREADS_NUMBERS); do \
			  	echo '\n'; \
				ARGS=`expr $(FILE_NAME)' '$$NO_THREADS' '$$TEST_METHOD' '$(SOURCES_DIR)/$$IMAGE_NAME' '$(RESULTS_DIR)/$$IMAGE_NAME`; \
				echo 'TEST: ./'$$ARGS; \
				./$$ARGS; \
			done; \
		done; \
	done; \

clean:
	@rm -f $(FILE_NAME)

clean_all: clean
	@rm -f $(RESULTS_DIR)/*
	@rm -f $(TIMES_FILE_NAME)

# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Compiled files names
FORK_FILE_NAME=fork
EXEC_FILE_NAME=exec

# Targets names
TARGETS=$(FORK_FILE_NAME) $(EXEC_FILE_NAME)


all: clean $(TARGETS)

$(FORK_FILE_NAME):
	@$(CC) $(C_FLAGS) $(FORK_FILE_NAME).c -o $(FORK_FILE_NAME)

$(EXEC_FILE_NAME):
	@$(CC) $(C_FLAGS) $(EXEC_FILE_NAME).c -o $(EXEC_FILE_NAME)

clean:
	@rm -f $(FORK_FILE_NAME)
	@rm -f $(EXEC_FILE_NAME)

# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Compiled file name
COMP_FILE_NAME=main.c
# Output file name
OUT_FILE=main

# Targets names
TARGETS=$(OUT_FILE)


all: clean $(TARGETS)

$(OUT_FILE):
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -o $(OUT_FILE)

clean:
	@rm -f $(OUT_FILE)

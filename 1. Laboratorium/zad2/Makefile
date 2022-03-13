	# Used compiler
CC=gcc
# Included directories path
INC_DIRS=../zad1
# Libraries directories
LIB_DIR=../zad1
# Compiler options
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -I $(INC_DIRS) -L $(LIB_DIR) $(C_OPT) -D MEASURE_TIME

# Library name
LIB_NAME=sysops
# Executable files prefix
EXE_PREFIX=tests_
# Report file name
REPORT_NAME=raport2.txt

# Targets names
TARGETS=static_tests shared_tests dynamic_tests


all: $(TARGETS)

static:
	make -C $(LIB_DIR) static

shared:
	make -C $(LIB_DIR) shared

clean:
	rm -f $(REPORT_NAME)
	rm -f *_tests

clean_all: clean
	make -C $(LIB_DIR) clean_all

static_tests: static
	$(CC) $(C_FLAGS) -static main.c -l $(LIB_NAME) -o $(EXE_PREFIX)static

shared_tests: shared
	$(CC) $(C_FLAGS) -fPIC -shared main.c -o $(EXE_PREFIX)shared

dynamic_tests: shared
	$(CC) $(C_FLAGS) -D DYNAMIC_MODE main.c -ldl -o $(EXE_PREFIX)dynamic

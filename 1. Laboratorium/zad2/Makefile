	# Used compiler
CC=gcc
# Included directories path
INC_DIRS=../zad1
# Libraries directories
LIB_DIRS=../zad1
# Compiler options
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -I$(INC_DIRS) $(C_OPT)

# Library name
LIB_NAME=mylib
# Report file name
REPORT_NAME=raport2.txt

# Targets names
TARGETS=test


all: $(TARGETS)

static:
	make -C ../zad1 static
	$(CC) $(CFLAGS) main.c -o test-static -L$(LIB_DIRS) $(LIB_NAME).a #FIXME

shared:
	make -C ../zad1 shared
	$(CC) $(CFLAGS) main.c -o test-shared -L$(LIB_DIRS) $(LIB_NAME).so #FIXME

clean:
	rm -f *.o

clean_all: clean
	rm -f $(REPORT_NAME)
	make -C ../zad1 clean_all

test: static shared
	#TODO

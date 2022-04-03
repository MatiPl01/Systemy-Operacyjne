# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Names of the files that should be compiled
FILES_NAMES=catcher sender

# Targets names
TARGETS=$(OUT_FILE)


all: compile

compile:
	@COMP_NAME=''; \
	for OUT_NAME in $(FILES_NAMES); do \
		COMP_NAME=`expr $$OUT_NAME.`c; \
		$(CC) $(C_FLAGS) $$COMP_NAME -o $$OUT_NAME; \
	done; \

clean:
	@for OUT_NAME in $(FILES_NAMES); do \
		rm -f $$OUT_NAME; \
	done; \

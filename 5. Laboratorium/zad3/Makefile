# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall  -std=gnu11 -g $(C_OPT)

# File names
FILE_NAMES=main consumer producer

# Targets names
TARGETS=$(FILE_NAMES)


all: compile

.PHONY: compile
compile:
	@COMP_NAME=''; \
	for OUT_NAME in $(FILE_NAMES); do \
		COMP_NAME=`expr $$OUT_NAME`.c; \
		$(CC) $(C_FLAGS) $$COMP_NAME -o $$OUT_NAME; \
	done; \

clean:
	@rm -f $(FILE_NAMES)

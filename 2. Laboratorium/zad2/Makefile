# Used compiler
CC=gcc
# Libraries directories
LIB_DIR=./libraries
# Compiler optimization
C_OPT=-O0
# Declaration
DECLARATION=_
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -L $(LIB_DIR) $(C_OPT) -D $(DECLARATION)

# Compiled file name
COMP_FILE_NAME=main.c

# Output files names
COUNT_LIB_NAME=countlib
COUNT_SYS_NAME=countsys

# Characters for subsequent tests
TESTS_CHARS=a g h

# Files directory
FILES_DIR=./files
# Input file prefix
INPUT_FILE_PREFIX=$(FILES_DIR)/input

# Time measurement results file path
TIME_FILE_PATH=pomiar_zad_2.txt
# Targets names
TARGETS=$(COUNT_LIB_NAME) $(COUNT_SYS_NAME)


all: clean $(TARGETS)

static:
	@make -C $(LIB_DIR) $(LIB_NAME)_static

$(COUNT_LIB_NAME):
	@rm -f $(COUNT_LIB_NAME)
	@make static LIB_NAME=$(COUNT_LIB_NAME)
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COUNT_LIB_NAME) -o $(COUNT_LIB_NAME)

$(COUNT_SYS_NAME):
	@rm -f $(COUNT_SYS_NAME)
	@make static LIB_NAME=$(COUNT_SYS_NAME)
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COUNT_SYS_NAME) -o $(COUNT_SYS_NAME) -D LIB_SYS

tests: clean_all
	@make $(COPY_LIB_NAME) DECLARATION=MEASURE_TIME='\"$(TIME_FILE_PATH)\"'
	@make $(COPY_SYS_NAME) DECLARATION=MEASURE_TIME='\"$(TIME_FILE_PATH)\"'
	@i=1; \
	for CHAR in $(TESTS_CHARS); do \
		make run_test NUM=$$i CHAR=$$CHAR INPUT_FILE=$(INPUT_FILE_PREFIX)-$$i.txt; \
		i=`expr $$i + 1`; \
	done; \
	make clean

run_test:
	$(call write_line,"=================== Test $(NUM) ===================")
	$(call write_line,"Counted character: $(CHAR)")
	$(call write_line,"Input file: $(INPUT_FILE)")
	$(call write_line,"")
	echo ''
	$(call test_lib,$(COUNT_LIB_NAME),$(CHAR),$(INPUT_FILE))
	$(call write_line,"")
	echo ''
	$(call test_lib,$(COUNT_SYS_NAME),$(CHAR),$(INPUT_FILE))
	echo ''
	$(call write_line,"")

clean:
	@rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a $(COUNT_LIB_NAME) $(COUNT_SYS_NAME)

clean_all: clean
	@rm -f $(TIME_FILE_PATH)

define write_line
	@echo $1 >> $(TIME_FILE_PATH)
endef

define test_lib
	$(call write_line,"Results for $1:")
	./$1 $2 $3
endef

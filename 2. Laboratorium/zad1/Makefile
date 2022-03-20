# Used shell
SHELL=/bin/bash
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

# Executable files names
COPY_LIB_NAME=copylib
COPY_SYS_NAME=copysys

# Number of input files for the time measurement
INPUT_FILES_NUM=4

# Files directory
FILES_DIR=./files
# Input file prefix
INPUT_FILE_PREFIX=$(FILES_DIR)/input
# Output file prefix
OUTPUT_FILE_PREFIX=$(FILES_DIR)/output

# Time measurement results file path
TIME_FILE_PATH=pomiar_zad_1.txt

# Targets names
TARGETS=$(COPY_LIB_NAME) $(COPY_SYS_NAME)


all: clean $(TARGETS)

static:
	make -C $(LIB_DIR) $(LIB_NAME)_static

$(COPY_LIB_NAME):
	rm -f $(COPY_LIB_NAME)
	make static LIB_NAME=$(COPY_LIB_NAME)
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COPY_LIB_NAME) -o $(COPY_LIB_NAME)

$(COPY_SYS_NAME):
	rm -f $(COPY_SYS_NAME)
	make static LIB_NAME=$(COPY_SYS_NAME)
	$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(COPY_SYS_NAME) -o $(COPY_SYS_NAME) -D LIB_SYS

tests: clean_all
	make $(COPY_LIB_NAME) DECLARATION=MEASURE_TIME='\"$(TIME_FILE_PATH)\"'
	make $(COPY_SYS_NAME) DECLARATION=MEASURE_TIME='\"$(TIME_FILE_PATH)\"'
	i=1; \
 	while [[ $$i -le $(INPUT_FILES_NUM) ]]; do \
  		make run_test NUM=$$i INPUT_FILE=$(INPUT_FILE_PREFIX)-$$i.txt OUTPUT_FILE=$(OUTPUT_FILE_PREFIX)-$$i.txt; \
  		i=`expr $$i + 1`; \
  	done; \
  	make clean

run_test:
	$(call write_line,"=================== Test $(NUM) ===================")
	$(call write_line,"Input file: $(INPUT_FILE)")
	$(call write_line,"Output file: $(OUTPUT_FILE)")
	$(call write_line,"")
	$(call test_lib,$(COPY_LIB_NAME),$(INPUT_FILE),$(OUTPUT_FILE))
	$(call write_line,"")
	$(call test_lib,$(COPY_SYS_NAME),$(INPUT_FILE),$(OUTPUT_FILE))
	$(call write_line,"")

clean:
	rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a $(COPY_LIB_NAME) $(COPY_SYS_NAME)

clean_all: clean
	rm -f $(TIME_FILE_PATH) $(OUTPUT_FILE_PREFIX)*.txt

define write_line
	echo $1 >> $(TIME_FILE_PATH)
endef

define test_lib
	$(call write_line,"Results for $1:")
	./$1 $2 $3
endef

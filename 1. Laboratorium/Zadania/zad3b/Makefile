# Used compiler
CC=gcc
# Included directories path
INC_DIRS=../zad1
# Libraries directories
LIB_DIR=../zad1
# Compiler optimizations list
C_OPT_LIST=-O0 -O1 -O2 -O3 -Os
# Default optimization level
C_OPT=-O0
# Declarations
DECLARATIONS=_
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -I $(INC_DIRS) -L $(LIB_DIR) -D $(DECLARATIONS)

# Compiled file name
COMP_FILE_NAME=../zad2/main.c
# Library name
LIB_NAME=sysops
# Output files prefix
OUT_PREFIX=tests_
# Input files path
INPUT_PATH=../input
# Report file name
REPORT_PATH=raport3b.txt

# Targets names
TARGETS=$(OUT_PREFIX)static $(OUT_PREFIX)shared $(OUT_PREFIX)dynamic


all: $(TARGETS)

static:
	@make -C $(LIB_DIR) static C_OPT=-O0

shared:
	@make -C $(LIB_DIR) shared C_OPT=-O0

clean:
	@rm -f $(REPORT_PATH)
	@rm -f $(OUT_PREFIX)*

clean_all: clean
	@make -C $(LIB_DIR) clean_all

$(OUT_PREFIX)static: static
	@$(CC) $(C_FLAGS) $(C_OPT) $(COMP_FILE_NAME) -static -l $(LIB_NAME) -o $(OUT_PREFIX)static

$(OUT_PREFIX)shared: shared
	@$(CC) $(C_FLAGS) $(C_OPT) $(COMP_FILE_NAME) -Wl,-R $(LIB_DIR) -l $(LIB_NAME) -o $(OUT_PREFIX)shared

$(OUT_PREFIX)dynamic: shared
	@$(CC) $(C_FLAGS) $(C_OPT) $(COMP_FILE_NAME) -D DYNAMIC_MODE -ldl -o $(OUT_PREFIX)dynamic

report: clean
	@for OPT in $(C_OPT_LIST); do \
  		make all C_OPT=$$OPT DECLARATIONS=MEASURE_TIME=0; \
  		make run_tests C_OPT=$$OPT; \
  	done

run_tests:
	$(call write_header,"Tests for $(C_OPT) optimization level")
	$(call write_line,"============ Static library tests ===========")
	$(call run_tests_helper,$(OUT_PREFIX)static)
	$(call write_line,"\n============ Shared library tests ===========")
	$(call run_tests_helper,$(OUT_PREFIX)shared)
	$(call write_line,"\n============ Dynamic library tests ==========")
	$(call run_tests_helper,$(OUT_PREFIX)dynamic)
	$(call write_line,"\n\n")

define write_header
	$(call write_line,"*********************************************")
	$(call write_line,$1)
	$(call write_line,"*********************************************\n")
endef

define write_line
	@echo $1 | tee -a $(REPORT_PATH)
endef

define run_tests_helper
	$(call run_test,$1,test-1,"\n1. Test:\n\t- 2 small files\n\t- Table size: 1\n\t- 100x create 1 block / remove 1 block\n\t- 2 files per block\n")
	$(call run_test,$1,test-2,"\n2. Test:\n\t- 3 medium files\n\t- Table size: 5\n\t- 20x create 5 blocks / remove 5 blocks\n\t- 9 files per block\n")
	$(call run_test,$1,test-3,"\n3. Test:\n\t- 3 large files\n\t- Table size: 10\n\t- create 10 blocks\n\t- 2x create remove 5 blocks / recreate 5 blocks\n\t- 20 files per block\n")
	$(call run_test,$1,test-4,"\n4. Test:\n\t- 2 huge files\n\t- Table size: 3\n\t- create / remove up to 3 blocks (random order)\n\t- 5 files per block\n")
endef

define run_test
	$(call write_line,$3)
    @./$1 `cat $(INPUT_PATH)/$2.txt | tr '\r\n' ' '` | tee -a $(REPORT_PATH)
endef

# Used compiler
CC=gcc
# Included directories path
INC_DIRS=../zad1
# Libraries directories
LIB_DIR=../zad1
# Compiler optimization
C_OPT=-O0
# Declarations
DECLARATIONS=_
# Compiler flags
C_FLAGS=-Wall -Wextra -Werror -std=gnu11 -g -I $(INC_DIRS) -L $(LIB_DIR) $(C_OPT) -D $(DECLARATIONS)

# Compiled file name
COMP_FILE_NAME=../zad2/main.c
# Library name
LIB_NAME=sysops
# Output file name
OUT_NAME=tests
# Input files path
INPUT_PATH=../input
# Report file name
REPORT_PATH=raport2.txt

# Targets names
TARGETS=tests


all: $(TARGETS)

static:
	@make -C $(LIB_DIR) static

clean:
	@rm -f $(REPORT_PATH)
	@rm -f $(OUT_NAME)

clean_all: clean
	@make -C $(LIB_DIR) clean_all

$(OUT_NAME): static
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(LIB_NAME) -o $(OUT_NAME)

report: clean
	@make all DECLARATIONS=MEASURE_TIME=1
	@$(call write_line,"=================== Tests ===================")
	@$(call run_tests,$(OUT_NAME))

define write_line
	@echo $1 | tee -a $(REPORT_PATH)
endef

define run_tests
	$(call run_test,$1,test-1,"\n1. Test:\n\t- 2 small files\n\t- Table size: 1\n\t- 100x create 1 block / remove 1 block\n\t- 2 files per block\n")
	$(call run_test,$1,test-2,"\n2. Test:\n\t- 3 medium files\n\t- Table size: 5\n\t- 20x create 5 blocks / remove 5 blocks\n\t- 9 files per block\n")
	$(call run_test,$1,test-3,"\n3. Test:\n\t- 3 large files\n\t- Table size: 10\n\t- create 10 blocks\n\t- 2x create remove 5 blocks / recreate 5 blocks\n\t- 20 files per block\n")
	$(call run_test,$1,test-4,"\n4. Test:\n\t- 2 huge files\n\t- Table size: 3\n\t- create / remove up to 3 blocks (random order)\n\t- 5 files per block\n")
endef

define run_test
	$(call write_line,$3)
    @./$1 `cat $(INPUT_PATH)/$2.txt | tr '\r\n' ' '` | tee -a $(REPORT_PATH)
endef

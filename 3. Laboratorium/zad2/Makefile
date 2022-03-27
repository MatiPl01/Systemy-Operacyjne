# Used compiler
CC=gcc
# Library directory
LIB_DIR=./library
# Compiler optimization
C_OPT=-O0
# Declaration
DECLARATION=_
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g -L $(LIB_DIR) $(C_OPT) -D $(DECLARATION)

# Compiled file name
COMP_FILE_NAME=main.c
# Output file name
OUT_FILE=main
# Library file name
LIB_NAME=integrate

# Time measurement results file path
REPORT_FILE_PATH=pomiar_zad_2.txt

# Targets names
TARGETS=$(OUT_FILE)


all: clean_all $(TARGETS)

static:
	@make -C $(LIB_DIR) $(LIB_NAME)_static

$(OUT_FILE):
	@make static LIB_NAME=$(LIB_NAME)
	@$(CC) $(C_FLAGS) $(COMP_FILE_NAME) -static -l $(LIB_NAME) -o $(OUT_FILE)

tests: clean_all
	@make $(OUT_FILE) DECLARATION=MEASURE_TIME='\"$(REPORT_FILE_PATH)\"'
	$(call run_tests)
	@make clean

clean:
	@rm -f $(LIB_DIR)/*.o $(LIB_DIR)/*.a $(OUT_FILE)

clean_all: clean
	@rm -f $(OUT_FILE) $(REPORT_FILE_PATH)

define write_line
	@echo $1 | tee -a $(REPORT_FILE_PATH)
endef

define run_tests
	$(call run_test,.000000005,1)
	$(call run_test,.000000005,2)
	$(call run_test,.000000005,3)
	$(call run_test,.000000005,5)
	$(call run_test,.000000005,10)
	$(call run_test,.0000000005,1000)
endef

define run_test
	$(call write_line,"=================== Test ===================")
	$(call write_line,"Rectangle width:     $1")
	$(call write_line,"Number of processes: $2")
	$(call write_line,"")
    @./$(OUT_FILE) $1 $2
    $(call write_line,"")
endef

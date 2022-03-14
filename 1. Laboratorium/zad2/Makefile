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
REPORT_PATH=raport2.txt

# Targets names
TARGETS=$(EXE_PREFIX)static $(EXE_PREFIX)shared $(EXE_PREFIX)dynamic


all: $(TARGETS)

static:
	make -C $(LIB_DIR) static

shared:
	make -C $(LIB_DIR) shared

clean:
	rm -f $(REPORT_PATH)
	rm -f *_tests

clean_all: clean
	make -C $(LIB_DIR) clean_all

$(EXE_PREFIX)static: static
	$(CC) $(C_FLAGS) -static main.c -l $(LIB_NAME) -o $(EXE_PREFIX)static

#FIXME
$(EXE_PREFIX)shared: shared
	$(CC) $(C_FLAGS) -shared main.c -fPIC -o $(EXE_PREFIX)shared

$(EXE_PREFIX)dynamic: shared
	$(CC) $(C_FLAGS) -D DYNAMIC_MODE main.c -ldl -o $(EXE_PREFIX)dynamic

report: clean all
	echo "============ Static library tests ===========" >> $(REPORT_PATH)
	$(call run_tests,$(EXE_PREFIX)static)
	echo "\n============ Shared library tests ===========" >> $(REPORT_PATH)
	$(call run_tests,$(EXE_PREFIX)shared)
	echo "\n============ Dynamic library tests ==========" >> $(REPORT_PATH)
	$(call run_tests,$(EXE_PREFIX)dynamic)

define run_tests
	echo "1 small file" >> $(REPORT_PATH)
	$(call run_test,$1,test-1)
endef

define run_test
    ./$(1) `cat ./input/$(2).txt | tr '\r\n' ' '` >> $(REPORT_PATH)
endef

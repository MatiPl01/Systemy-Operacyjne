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
# Output files prefix
OUT_PREFIX=tests_
# Report file name
REPORT_PATH=raport2.txt

# Targets names
TARGETS=$(OUT_PREFIX)static $(OUT_PREFIX)shared $(OUT_PREFIX)dynamic


all: $(TARGETS)

static:
	make -C $(LIB_DIR) static

shared:
	make -C $(LIB_DIR) shared

clean:
	rm -f $(REPORT_PATH)
	rm -f $(OUT_PREFIX)*

clean_all: clean
	make -C $(LIB_DIR) clean_all

$(OUT_PREFIX)static: static
	$(CC) $(C_FLAGS) main.c -l $(LIB_NAME) -o $(OUT_PREFIX)static

$(OUT_PREFIX)shared: shared
	$(CC) $(C_FLAGS) main.c -Wl,-R $(LIB_DIR) -l $(LIB_NAME) -o $(OUT_PREFIX)shared

$(OUT_PREFIX)dynamic: shared
	$(CC) $(C_FLAGS) main.c -D DYNAMIC_MODE -ldl -o $(OUT_PREFIX)dynamic

report: clean all
	$(call write_line,"============ Static library tests ===========")
	$(call run_tests,$(OUT_PREFIX)static)
	$(call write_line,"\n============ Shared library tests ===========")
	$(call run_tests,$(OUT_PREFIX)shared)
	$(call write_line,"\n============ Dynamic library tests ==========")
	$(call run_tests,$(OUT_PREFIX)dynamic)

define write_line
	echo $1 >> $(REPORT_PATH)
endef

define run_tests
	$(call run_test,$1,test-1,"1 small file")
endef

define run_test
	$(call write_line,$3)
    ./$1 `cat ./input/$2.txt | tr '\r\n' ' '` >> $(REPORT_PATH)
endef

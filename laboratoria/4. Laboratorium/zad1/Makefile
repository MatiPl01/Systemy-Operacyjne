# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Fork file directory path
FORK_DIR_PATH=./fork

# Exec file directory path
EXEC_DIR_PATH=./exec

# Report file name
REPORT_FILE_NAME=raport1.txt
# Report file path relative to the fork directory
REPORT_FORK_REL_PATH=..
# Report file path relative to the exec directory
REPORT_EXEC_REL_PATH=..

# Targets names
TARGETS=fork exec


all: clean_all $(TARGETS)

.PHONY: fork
fork:
	@make -C $(FORK_DIR_PATH)

.PHONY: exec
exec:
	@make -C $(EXEC_DIR_PATH)

fork_tests: fork
	$(call call_fork_tests)
	@make clean

exec_tests: exec
	$(call call_exec_tests)
	@make clean

tests: clean_all
	$(call call_fork_tests)
	$(call write_line,"")
	$(call call_exec_tests)
	@make clean

clean:
	@make -C $(FORK_DIR_PATH) clean
	@make -C $(EXEC_DIR_PATH) clean

clean_all:
	@rm -f $(REPORT_FILE_NAME)
	@make -C $(FORK_DIR_PATH) clean_all
	@make -C $(EXEC_DIR_PATH) clean_all


define write_line
	@echo $1 | tee -a $(REPORT_FILE_NAME)
endef

define write_header
	$(call write_line,"**********")
	$(call write_line,$1)
	$(call write_line,"**********\n")
endef

define call_fork_tests
	$(call write_header, "fork tests")
	@make -C $(FORK_DIR_PATH) tests_no_clean REPORT_FILE_PATH=$(REPORT_FORK_REL_PATH)/$(REPORT_FILE_NAME)
endef

define call_exec_tests
	$(call write_header, "exec tests")
	@make -C $(EXEC_DIR_PATH) tests_no_clean REPORT_FILE_PATH=$(REPORT_EXEC_REL_PATH)/$(REPORT_FILE_NAME)
endef

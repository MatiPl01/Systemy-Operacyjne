# Used compiler
CC=gcc
# Compiler optimization
C_OPT=-O0
# Compiler flags
C_FLAGS=-Wall -std=gnu11 -g $(C_OPT)

# Compiled file name
FILE_NAME=main

# Report file name
REPORT_FILE_PATH=raport-fork.txt

# Targets names
TARGETS=$(FILE_NAME)


all: clean_all $(TARGETS)

$(FILE_NAME):
	@$(CC) $(C_FLAGS) $(FILE_NAME).c -o $(FILE_NAME)

tests: clean_all tests_no_clean

tests_no_clean: $(TARGETS)
	$(call call_action,ignore)
	$(call call_action,handler)
	$(call call_action,mask)
	$(call call_action,pending)

clean:
	@rm -f $(FILE_NAME)

clean_all: clean
	@rm -f $(REPORT_FILE_PATH)


define call_action
	@./$(FILE_NAME) $1 | tee -a $(REPORT_FILE_PATH)
	$(call write_line,"")
endef

define write_line
	@echo $1 | tee -a $(REPORT_FILE_PATH)
endef

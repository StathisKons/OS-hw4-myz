BUILD_DIR := ./build

SRC_DIRS := ./src
COMMON_SRCS := $(shell find $(SRC_DIRS) -name '*.c')
COMMON_OBJS := $(COMMON_SRCS:%.c=$(BUILD_DIR)/%.o)

INC_DIRS := $(shell find ./include -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC := gcc
CFLAGS := -g3 -Wall -Wextra -pedantic # -Werror -O3
LDFLAGS := 

PROGRAM := myz


# ARGS := -s $(SHMID) -c 20 -o $(ORDER_TIME) -r $(REST_TIME)
ARCHIVE_FILE := test_cases.myz
LIST_OF_FILES_DIRS := test_cases temp


# Default target
.PHONY: all
all: $(PROGRAM)

# Program-specific source and object files
myz: $(COMMON_OBJS) $(BUILD_DIR)/programs/myz.o
	$(CC) $^ -o $@ $(LDFLAGS) 


# Compile program-specific source files
$(BUILD_DIR)/programs/%.o: ./programs/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

# Compile common source files
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

# Run targets
.PHONY: run
	# @echo "Specify a program to run: make run PROGRAM=builder|lexan|splitter"
	# @./$(BUILD_DIR)/$(PROGRAM) $(ARGS)
run: $(PROGRAM)
	./$(PROGRAM) $(ARGS)

.PHONY run_create
run_create: $(PROGRAM)
	./$(PROGRAM) -c $(ARCHIVE_FILE) $(LIST_OF_FILES_DIRS)

.PHONY run_append
run_append: $(PROGRAM)
	./$(PROGRAM) -a $(ARCHIVE_FILE) $(LIST_OF_FILES_DIRS)

.PHONY run_extract
run_extract: $(PROGRAM)
	./$(PROGRAM) -x $(ARCHIVE_FILE)

.PHONY run_delete
run_delete: $(PROGRAM)
	./$(PROGRAM) -d $(ARCHIVE_FILE) $(LIST_OF_FILES_DIRS)

.PHONY run_metadata
run_metadata: $(PROGRAM)
	./$(PROGRAM) -m $(ARCHIVE_FILE)

.PHONY run_query
run_query: $(PROGRAM)
	./$(PROGRAM) -q $(ARCHIVE_FILE) $(LIST_OF_FILES_DIRS)

.PHONY run_print
run_print: $(PROGRAM)
	./$(PROGRAM) -p $(ARCHIVE_FILE)

.PHONY run_create_compressed
run_create_compressed: $(PROGRAM)
	./$(PROGRAM) -jc $(ARCHIVE_FILE) $(LIST_OF_FILES_DIRS)

.PHONY run_append_compressed
run_append_compressed: $(PROGRAM)
	./$(PROGRAM) -ja $(ARCHIVE_FILE) $(LIST_OF_FILES_DIRS)

.PHONY: val
val: $(PROGRAM)
	valgrind --trace-children=yes --leak-check=full ./$(PROGRAM) $(ARGS)

# Clean
.PHONY: clean
clean:
	rm -f $(PROGRAMS)
	rm -rf $(BUILD_DIR)
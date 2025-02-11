BUILD_DIR := ./build

SRC_DIRS := ./src
COMMON_SRCS := $(shell find $(SRC_DIRS) -name '*.c')
COMMON_OBJS := $(COMMON_SRCS:%.c=$(BUILD_DIR)/%.o)

INC_DIRS := ./include # ./include/data_structures ./include/utils ./include/programs
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC := gcc
CFLAGS := -g3 -Wall -Wextra -pedantic # -Werror -O3
LDFLAGS := -lm -pthread -lrt

PROGRAMS := initializer monitor visitor receptionist close_bar


SHMID = shm_for_nemea_sdi2300099
REST_TIME = 2
ORDER_TIME = 3
ARGS := -s $(SHMID) -c 20 -o $(ORDER_TIME) -r $(REST_TIME)


# Default target
.PHONY: all
all: $(PROGRAMS)

# Program-specific source and object files
initializer: $(COMMON_OBJS) $(BUILD_DIR)/programs/initializer.o
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
run: $(PROGRAMS)
	./initializer $(ARGS)

.PHONY: run_monitor
run_monitor: $(PROGRAMS)
	watch -d -n 0.5 --color ./monitor -s $(SHMID)

.PHONY: val
val: $(PROGRAMS)
	valgrind --trace-children=yes --leak-check=full ./initializer $(ARGS)

# Clean
.PHONY: clean
clean:
	rm -f $(PROGRAMS)
	rm -rf $(BUILD_DIR)
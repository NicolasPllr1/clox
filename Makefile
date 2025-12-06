CC := clang
NAME := clox 
BUILD_DIR := build

CFLAGS += -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter

# If we're building at a point in the middle of a chapter, don't fail if there
# are functions that aren't used yet.
ifeq ($(SNIPPET),true)
	CFLAGS += -Wno-unused-function
endif

# Files.
HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)
OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o)))

# Targets ---------------------------------------------------------------------

.PHONY: run all clean

run: all
	./clox

# Default target: builds the program
all: $(BUILD_DIR)/$(NAME)

# Link the interpreter.
$(BUILD_DIR)/$(NAME): $(OBJECTS) | $(BUILD_DIR) # Make sure BUILD_DIR exists before linking
	@ printf "%8s %-40s %s\n" "LD" $@ "$(CFLAGS)"
	@ $(CC) $(CFLAGS) $^ -o $@
	@ cp build/clox clox # For convenience, copy the interpreter to the top level.

# Compile object files.
$(BUILD_DIR)/%.o: %.c $(HEADERS) | $(BUILD_DIR) # Make sure BUILD_DIR exists before compiling
	@ printf "%8s %-40s %s\n" "CC" $< "$(CFLAGS)"
	@ $(CC) -c $(CFLAGS) -o $@ $<


# Rule to create the build directory
$(BUILD_DIR):
	@ mkdir -p $(BUILD_DIR)

# Remove all build outputs and intermediate files.
clean:
	@ rm -rf $(BUILD_DIR)

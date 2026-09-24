CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
BUILD   = build

all: $(BUILD)/scanner $(BUILD)/target_program

$(BUILD)/scanner: src/main.c src/memory_scanner.c include/memory_scanner.h | $(BUILD)
	$(CC) $(CFLAGS) src/main.c src/memory_scanner.c -o $@

$(BUILD)/target_program: src/target_program.c | $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: all clean

$(BUILD)/test_lists: test/test_lists.c src/memory_scanner.c include/memory_scanner.h | $(BUILD)
	$(CC) $(CFLAGS) test/test_lists.c src/memory_scanner.c -o $@

test: all $(BUILD)/test_lists
	./$(BUILD)/test_lists
	sudo ./test/integration.sh

.PHONY: test
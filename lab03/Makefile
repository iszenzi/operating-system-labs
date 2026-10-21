CC = gcc
CPPFLAGS += -D_GNU_SOURCE -Iinclude
CFLAGS = -std=c17 -O2 -g -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes
LDLIBS += -pthread
BUILD = build
OBJECTS = $(BUILD)/main.o $(BUILD)/game.o $(BUILD)/options.o

.PHONY: all run trace benchmark clean
all: $(BUILD)/dice

$(BUILD)/dice: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $@

run: all
	./$(BUILD)/dice --threads 4 10 3 21 18 100000

trace: all
	mkdir -p artifacts
	strace -f -e trace=clone,clone3,futex,gettid,exit,exit_group -o artifacts/trace.log ./$(BUILD)/dice --threads 4 20 0 0 0 100000 > artifacts/results.txt 2> artifacts/threads.log

benchmark: all
	python3 scripts/benchmark.py

clean:
	rm -rf -- $(BUILD)

-include $(OBJECTS:.o=.d)

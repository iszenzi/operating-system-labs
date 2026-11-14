CC = gcc
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -Iinclude
CFLAGS = -std=c17 -O2 -g -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes
BUILD = build
PARENT_OBJECTS = $(BUILD)/parent.o $(BUILD)/mapping.o $(BUILD)/io.o
CHILD_OBJECTS = $(BUILD)/child.o $(BUILD)/sum.o $(BUILD)/mapping.o $(BUILD)/io.o
OBJECTS = $(sort $(PARENT_OBJECTS) $(CHILD_OBJECTS))

.PHONY: all run trace clean
all: $(BUILD)/parent $(BUILD)/child

$(BUILD)/parent: $(PARENT_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/child: $(CHILD_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $@

run: all
	./$(BUILD)/parent

trace: all
	mkdir -p artifacts
	printf '%s\n' examples/numbers.txt | strace -f -yy -s 128 -e trace=%process,%desc,%memory,%signal -o artifacts/trace.log ./$(BUILD)/parent > artifacts/results.txt 2> artifacts/processes.log

clean:
	rm -rf -- $(BUILD)

-include $(OBJECTS:.o=.d)

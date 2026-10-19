CC = gcc
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -Iinclude
CFLAGS = -std=c17 -O2 -g -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes
DEPFLAGS = -MMD -MP

BUILD = build
PARENT_OBJECTS = $(BUILD)/parent.o $(BUILD)/io.o
CHILD_OBJECTS = $(BUILD)/child.o $(BUILD)/sum.o $(BUILD)/io.o
OBJECTS = $(sort $(PARENT_OBJECTS) $(CHILD_OBJECTS))

.PHONY: all run clean

all: $(BUILD)/parent $(BUILD)/child

$(BUILD)/parent: $(PARENT_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/child: $(CHILD_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

run: all
	./$(BUILD)/parent

clean:
	rm -rf -- $(BUILD)

-include $(OBJECTS:.o=.d)

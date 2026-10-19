CC = gcc
CXX = g++
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -Iinclude
CFLAGS = -std=c17 -O2 -g -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes
CXXFLAGS = -std=c++17 -O2 -g -Wall -Wextra -Wpedantic
DEPFLAGS = -MMD -MP

BUILD = build
PARENT_OBJECTS = $(BUILD)/parent.o $(BUILD)/io.o
CHILD_OBJECTS = $(BUILD)/child.o $(BUILD)/sum.o $(BUILD)/io.o
OBJECTS = $(sort $(PARENT_OBJECTS) $(CHILD_OBJECTS))

.PHONY: all run test trace clean

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

$(BUILD)/test_lab02: tests/test_lab02.cpp $(BUILD)/sum.o include/sum.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) tests/test_lab02.cpp $(BUILD)/sum.o -lgtest_main -lgtest -pthread $(LDLIBS) -o $@

test: all $(BUILD)/test_lab02
	./$(BUILD)/test_lab02

trace: all
	mkdir -p artifacts
	printf '%s\n' examples/numbers.txt | strace -f -yy -s 128 -e trace=%process,%desc,%signal -o artifacts/trace.log ./$(BUILD)/parent > artifacts/results.txt 2> artifacts/processes.log
	@printf '%s\n' 'Логи сохранены в lab02/artifacts/'

clean:
	rm -rf -- $(BUILD)

-include $(OBJECTS:.o=.d)

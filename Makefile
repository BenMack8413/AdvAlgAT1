# Compiler and flags
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc

# Custom Output Directories
BUILD_DIR = build
BIN_DIR   = bin

# Target executable location
TARGET   = $(BIN_DIR)/regex_tool

# Source files
SRCS     = src/main.cpp \
           src/RegexApp.cpp \
           src/Input/InputHandler.cpp \
           src/Input/FileReader.cpp \
           src/Logic/RegexMatcher.cpp \
           src/Output/OutputFormatter.cpp

OBJS     = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run test suite
test: $(TARGET)
	@bash tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all test clean
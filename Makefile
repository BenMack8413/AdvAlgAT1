# Compiler and flags
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.

# Target executable
TARGET   = regex_tool

# Source files located across subdirectories
SRCS     = main.cpp \
           RegexApp.cpp \
           Input/InputHandler.cpp \
           Input/FileReader.cpp \
           Logic/RegexMatcher.cpp \
           Output/OutputFormatter.cpp

OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
#pragma once
#include <string>
#include <vector>
#include <stdexcept>

struct RegexConfig {
    bool case_insensitive = false;
    bool invert_match = false;
    bool count_only = false;
    bool line_numbers = false;
    
    std::string pattern;
    std::vector<std::string> targets; 
};

class InputHandler {
public:
    // A single static method replaces the constructor and all getters
    static RegexConfig parse(int argc, char* argv[]);
};
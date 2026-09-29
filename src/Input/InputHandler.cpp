// InputHandler.cpp
#include "InputHandler.hpp"

RegexConfig InputHandler::parse(int argc, char* argv[]) {
    RegexConfig config;
    bool pattern_found = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // If it starts with '-', we haven't found the pattern yet, and it's not a standalone '-'
        if (!pattern_found && !arg.empty() && arg[0] == '-' && arg != "-") {
            for (size_t j = 1; j < arg.length(); ++j) {
                switch (arg[j]) {
                    case 'i': config.case_insensitive = true; break;
                    case 'v': config.invert_match = true; break;
                    case 'c': config.count_only = true; break;
                    case 'n': config.line_numbers = true; break;
                    default:
                        throw std::invalid_argument(std::string("Unknown flag: -") + arg[j]);
                }
            }
        } 
        // Otherwise, it is a positional argument (pattern or target)
        else {
            if (!pattern_found) {
                config.pattern = arg;
                pattern_found = true;
            } else {
                config.targets.push_back(arg);
            }
        }
    }

    if (config.pattern.empty()) {
        throw std::invalid_argument("Usage: regex_tool [FLAGS] <pattern> [target1 target2 ...]");
    }

    return config;
}
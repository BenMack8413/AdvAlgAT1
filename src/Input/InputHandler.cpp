#include "InputHandler.hpp"
#include <stdexcept>

RegexConfig InputHandler::parse(int argc, char* argv[]) {
    RegexConfig config;
    bool pattern_found = false;
    bool force_positional = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // 1. Check for the double-dash delimiter
        if (!force_positional && arg == "--") {
            force_positional = true;
            continue;
        }

        // 2. Parse as a flag IF: not forced positional, pattern not yet found, starts with '-', and isn't just "-"
        if (!force_positional && !pattern_found && !arg.empty() && arg[0] == '-' && arg != "-") {
            for (size_t j = 1; j < arg.length(); ++j) {
                switch (arg[j]) {
                    case 'i': config.case_insensitive = true; break;
                    case 'c': config.count_only = true; break;
                    default:
                        throw std::invalid_argument(std::string("Unknown flag: -") + arg[j]);
                }
            }
        } 
        // 3. Otherwise, parse as a positional argument (pattern or target)
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
        throw std::invalid_argument("Usage: regex_tool [FLAGS] [--] <pattern> [target1 target2 ...]");
    }

    return config;
}
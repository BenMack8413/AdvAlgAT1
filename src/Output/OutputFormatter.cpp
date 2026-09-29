#include "OutputFormatter.hpp"
#include <iostream>

void OutputFormatter::display_string_match(const std::string& text, const MatchResult& match) const {
    if (!match.matched) {
        return;
    }
    
    std::cout << text.substr(0, match.start_idx)
              << ANSI_RED_BOLD 
              << text.substr(match.start_idx, match.end_idx - match.start_idx) 
              << ANSI_RESET
              << text.substr(match.end_idx) 
              << "\n";
}

void OutputFormatter::display_file_match(size_t line_num, const std::string& line, const MatchResult& match) const {
    if (!match.matched) {
        return;
    }

    std::cout << "Line " << line_num << ": "
              << line.substr(0, match.start_idx)
              << ANSI_RED_BOLD 
              << line.substr(match.start_idx, match.end_idx - match.start_idx) 
              << ANSI_RESET
              << line.substr(match.end_idx) 
              << "\n";
}
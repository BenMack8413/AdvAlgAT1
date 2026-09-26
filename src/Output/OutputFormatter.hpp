#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP

#include <string>
#include <cstddef>
#include "Types.hpp"

class OutputFormatter {
public:
    static constexpr const char* ANSI_RED_BOLD = "\033[1;31m";
    static constexpr const char* ANSI_RESET = "\033[0m";

    void display_string_match(const std::string& text, const MatchResult& match) const;
    void display_file_match(size_t line_num, const std::string& line, const MatchResult& match) const;
};

#endif // OUTPUT_FORMATTER_HPP
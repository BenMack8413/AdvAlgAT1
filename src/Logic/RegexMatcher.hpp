#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <string>
#include "Types.hpp"

class RegexMatcher {
public:
    explicit RegexMatcher(const std::string& pattern);
    MatchResult find_match(const std::string& text) const;

private:
    std::string pattern_;
    void compile_pattern(const std::string& pattern);
};

#endif // REGEX_MATCHER_HPP
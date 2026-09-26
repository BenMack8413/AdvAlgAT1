#include "RegexMatcher.hpp"

RegexMatcher::RegexMatcher(const std::string& pattern) {
    compile_pattern(pattern);
}

MatchResult RegexMatcher::find_match(const std::string& text) const {
    MatchResult result;
    
    // TODO: Run Thompson NFA simulation across string indices
    // Populate result.matched, result.start_idx, and result.end_idx
    
    return result;
}

void RegexMatcher::compile_pattern(const std::string& pattern) {
    pattern_ = pattern;
    // TODO: Build AST and Thompson's NFA state machine graph
}
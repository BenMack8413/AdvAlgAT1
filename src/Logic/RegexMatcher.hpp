#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <string>
#include <unordered_set>
#include "Token.hpp"
#include "NfaBuilder.hpp"
#include "../Types.hpp" // Ensure this path matches your directory structure

class RegexMatcher {
public:
    explicit RegexMatcher(const std::string& pattern, bool case_insensitive = false);
    MatchResult find_match(const std::string& text) const;

private:
    std::string pattern_;
    NfaGraph nfa_;

    // Updated signature to handle ^ and $ anchors
    std::unordered_set<State*> get_epsilon_closure(const std::unordered_set<State*>& states, bool at_start, bool at_end) const;
};

#endif // REGEX_MATCHER_HPP
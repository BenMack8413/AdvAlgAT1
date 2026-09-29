#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include "Token.hpp"
#include "NfaBuilder.hpp"
#include "../Types.hpp"

class RegexMatcher {
public:
    explicit RegexMatcher(const std::string& pattern, bool case_insensitive = false);
    MatchResult find_match(const std::string& text) const;

private:
    std::string pattern_;
    NfaGraph nfa_;

    std::vector<State*> get_epsilon_closure(const std::vector<State*>& states, const std::string& text, size_t pos) const;
};

#endif // REGEX_MATCHER_HPP
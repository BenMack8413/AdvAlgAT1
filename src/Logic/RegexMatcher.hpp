#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <string>
#include <unordered_set>
#include "Token.hpp"
#include "NfaBuilder.hpp"
#include "../Types.hpp"

class RegexMatcher {
public:
    explicit RegexMatcher(const std::string& pattern);
    MatchResult find_match(const std::string& text) const;

private:
    std::string pattern_;
    NfaGraph nfa_;

    std::unordered_set<State*> get_epsilon_closure(const std::unordered_set<State*>& states) const;
};

#endif // REGEX_MATCHER_HPP
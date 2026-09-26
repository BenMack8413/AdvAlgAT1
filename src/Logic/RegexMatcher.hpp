#ifndef REGEX_MATCHER_HPP
#define REGEX_MATCHER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <stack>
#include "../Types.hpp"

// Single NFA Node
struct State {
    int id;
    bool is_accept = false;
    
    // Character transitions: char -> list of destination states
    std::unordered_map<char, std::vector<State*>> transitions;
    
    // Free transitions (epsilon)
    std::vector<State*> epsilon_transitions;
};

// Fragment representing an incomplete sub-NFA during construction
struct Fragment {
    State* start;
    State* accept;
};

class RegexMatcher {
public:
    explicit RegexMatcher(const std::string& pattern);
    MatchResult find_match(const std::string& text) const;

private:
    std::string pattern_;
    State* start_state_ = nullptr;
    State* accept_state_ = nullptr;
    
    // Memory owner for all state pointers
    std::vector<std::unique_ptr<State>> all_states_;
    int state_counter_ = 0;

    State* create_state();
    std::string insert_concat_operators(const std::string& pattern) const;
    std::string infix_to_postfix(const std::string& infix) const;
    void build_nfa(const std::string& postfix);
    
    std::unordered_set<State*> get_epsilon_closure(const std::unordered_set<State*>& states) const;
};

#endif // REGEX_MATCHER_HPP
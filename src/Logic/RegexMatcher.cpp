#include "RegexMatcher.hpp"
#include "Tokeniser.hpp" // Ensure this matches your file name (Tokeniser vs Tokeniser)
#include "Parser.hpp"
#include "NfaBuilder.hpp"
#include <stack>

RegexMatcher::RegexMatcher(const std::string& pattern, bool case_insensitive) {
    pattern_ = pattern;
    std::vector<Token> tokens = Tokeniser(pattern_, case_insensitive).tokenise();
    std::vector<Token> formatted = Parser::insert_concat_operators(tokens);
    std::vector<Token> postfix = Parser::infix_to_postfix(formatted);
    nfa_ = NfaBuilder::build_from_postfix(postfix);
}

std::unordered_set<State*> RegexMatcher::get_epsilon_closure(const std::unordered_set<State*>& states) const {
    std::unordered_set<State*> closure = states;
    std::stack<State*> stack;

    for (State* s : states) stack.push(s);

    while (!stack.empty()) {
        State* current = stack.top();
        stack.pop();

        for (State* next : current->epsilon_transitions) {
            if (closure.find(next) == closure.end()) {
                closure.insert(next);
                stack.push(next);
            }
        }
    }
    return closure;
}

MatchResult RegexMatcher::find_match(const std::string& text) const {
    MatchResult result;
    if (!nfa_.start) return result;

    // Left-most match evaluation: prioritize matches that start earlier in the string
    for (size_t start_pos = 0; start_pos <= text.length(); ++start_pos) {
        std::unordered_set<State*> current_states = get_epsilon_closure({nfa_.start});
        
        int longest_end = -1; // -1 indicates no match found for this start_pos

        // 1. Check for a zero-length match before consuming characters
        for (State* state : current_states) {
            if (state->is_accept) {
                longest_end = start_pos; 
            }
        }

        // 2. Consume characters and track the furthest accept state reached
        for (size_t i = start_pos; i < text.length(); ++i) {
            char c = text[i];
            std::unordered_set<State*> next_states;

            for (State* state : current_states) {
                for (const auto& trans : state->transitions) {
                    if (trans.matcher(c)) {
                        next_states.insert(trans.target);
                    }
                }
            }

            current_states = get_epsilon_closure(next_states);

            // If the NFA enters a dead state, stop consuming characters
            if (current_states.empty()) {
                break; 
            }

            // If we hit an accept state, record this as the new longest match
            for (State* state : current_states) {
                if (state->is_accept) {
                    longest_end = i + 1;
                }
            }
        }

        // 3. If a match was found, return it immediately (satisfies Left-Most, Longest rule)
        if (longest_end != -1) {
            result.matched = true;
            result.start_idx = start_pos;
            result.end_idx = longest_end;
            return result;
        }
    }

    return result;
}
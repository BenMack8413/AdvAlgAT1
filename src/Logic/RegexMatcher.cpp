#include "RegexMatcher.hpp"
#include "Tokeniser.hpp" // Ensure this matches your file name (Tokeniser vs Tokeniser)
#include "Parser.hpp"
#include "NfaBuilder.hpp"
#include <stack>

RegexMatcher::RegexMatcher(const std::string& pattern) {
    pattern_ = pattern;
    std::vector<Token> tokens = Tokeniser::tokenise(pattern_);
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

    for (size_t start_pos = 0; start_pos <= text.length(); ++start_pos) {
        std::unordered_set<State*> current_states = get_epsilon_closure({nfa_.start});

        // Check if starting state is an accept state before consuming characters
        for (State* state : current_states) {
            if (state->is_accept) {
                result.matched = true;
                result.start_idx = start_pos;
                result.end_idx = start_pos;
                // If text remaining, continue matching to find full substring match
            }
        }

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

            for (State* state : current_states) {
                if (state->is_accept) {
                    result.matched = true;
                    result.start_idx = start_pos;
                    result.end_idx = i + 1;
                    return result; // Return longest match starting at start_pos
                }
            }

            if (current_states.empty()) break;
        }

        if (result.matched) return result;
    }

    return result;
}
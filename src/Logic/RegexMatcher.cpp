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

std::unordered_set<State*> RegexMatcher::get_epsilon_closure(const std::unordered_set<State*>& states, bool at_start, bool at_end) const {
    std::unordered_set<State*> closure = states;
    std::stack<State*> stack;

    for (State* s : states) stack.push(s);

    while (!stack.empty()) {
        State* current = stack.top();
        stack.pop();

        // Block traversal if the anchor assertion condition fails
        if (current->anchor_assertion == Anchor::Start && !at_start) continue;
        if (current->anchor_assertion == Anchor::End && !at_end) continue;

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
        bool initial_at_start = (start_pos == 0);
        bool initial_at_end = (start_pos == text.length());
        
        // Pass boundary context to initial closure
        std::unordered_set<State*> current_states = get_epsilon_closure({nfa_.start}, initial_at_start, initial_at_end);
        
        int longest_end = -1;

        for (State* state : current_states) {
            if (state->is_accept) longest_end = start_pos; 
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

            // After consuming a character, we are never at the start boundary
            bool next_at_start = false; 
            bool next_at_end = (i + 1 == text.length());

            // Pass updated boundary context
            current_states = get_epsilon_closure(next_states, next_at_start, next_at_end);

            if (current_states.empty()) break; 

            for (State* state : current_states) {
                if (state->is_accept) longest_end = i + 1;
            }
        }

        if (longest_end != -1) {
            result.matched = true;
            result.start_idx = start_pos;
            result.end_idx = longest_end;
            return result;
        }
    }

    return result;
}
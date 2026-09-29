#include "RegexMatcher.hpp"
#include "Tokeniser.hpp"
#include "Parser.hpp"
#include "NfaBuilder.hpp"
#include <stack>
#include <cctype>
#include <limits>

RegexMatcher::RegexMatcher(const std::string& pattern, bool case_insensitive) {
    pattern_ = pattern;
    std::vector<Token> tokens = Tokeniser(pattern_, case_insensitive).tokenise();
    std::vector<Token> formatted = Parser::insert_concat_operators(tokens);
    std::vector<Token> postfix = Parser::infix_to_postfix(formatted);
    nfa_ = NfaBuilder::build_from_postfix(postfix);
}

std::vector<State*> RegexMatcher::get_epsilon_closure(const std::vector<State*>& states, const std::string& text, size_t pos) const {
    std::vector<State*> closure;
    std::unordered_set<State*> visited;
    std::stack<State*> stack;

    // Push in reverse order so the highest priority state is popped first
    for (auto it = states.rbegin(); it != states.rend(); ++it) {
        stack.push(*it);
    }

    bool at_start = (pos == 0);
    bool at_end = (pos == text.length());

    auto is_word_char = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };

    bool prev_word = (pos > 0) ? is_word_char(text[pos - 1]) : false;
    bool curr_word = (pos < text.length()) ? is_word_char(text[pos]) : false;
    bool at_wb = (prev_word != curr_word);

    while (!stack.empty()) {
        State* current = stack.top();
        stack.pop();

        if (!current || visited.count(current)) continue;

        // Verify boundary assertions
        if (current->anchor_assertion == Anchor::Start && !at_start) continue;
        if (current->anchor_assertion == Anchor::End && !at_end) continue;
        if (current->anchor_assertion == Anchor::WordBoundary && !at_wb) continue;
        if (current->anchor_assertion == Anchor::NonWordBoundary && at_wb) continue;

        visited.insert(current);
        if (current->is_accept || !current->transitions.empty()) {
            closure.push_back(current);
        }

        // Push transitions in reverse order to explore higher priority paths first
        for (auto it = current->epsilon_transitions.rbegin(); it != current->epsilon_transitions.rend(); ++it) {
            if (!visited.count(*it)) {
                stack.push(*it);
            }
        }
    }

    return closure;
}

MatchResult RegexMatcher::find_match(const std::string& text) const {
    MatchResult result;
    if (!nfa_.start) return result;

    for (size_t start_pos = 0; start_pos <= text.length(); ++start_pos) {
        std::vector<State*> current_states = get_epsilon_closure({nfa_.start}, text, start_pos);

        int best_end = -1;
        size_t best_priority = std::numeric_limits<size_t>::max();

        // Check if initial position matches
        for (size_t p = 0; p < current_states.size(); ++p) {
            if (current_states[p]->is_accept) {
                best_priority = p;
                best_end = start_pos;
                break;
            }
        }

        // Priority 0 accepted immediately (e.g. lazy question match at start)
        if (best_priority == 0) {
            result.matched = true;
            result.start_idx = start_pos;
            result.end_idx = best_end;
            return result;
        }

        for (size_t i = start_pos; i < text.length(); ++i) {
            char c = text[i];
            std::vector<State*> next_states;
            std::unordered_set<State*> next_visited;

            for (State* state : current_states) {
                for (const auto& trans : state->transitions) {
                    if (trans.matcher(c)) {
                        if (!next_visited.count(trans.target)) {
                            next_visited.insert(trans.target);
                            next_states.push_back(trans.target);
                        }
                    }
                }
            }

            current_states = get_epsilon_closure(next_states, text, i + 1);

            if (current_states.empty()) {
                break;
            }

            for (size_t p = 0; p < current_states.size(); ++p) {
                if (current_states[p]->is_accept) {
                    if (p <= best_priority) {
                        best_priority = p;
                        best_end = i + 1;
                    }
                    break;
                }
            }

            // Top priority thread reached accept state
            if (best_priority == 0) {
                break;
            }
        }

        if (best_end != -1) {
            result.matched = true;
            result.start_idx = start_pos;
            result.end_idx = best_end;
            return result;
        }
    }

    return result;
}
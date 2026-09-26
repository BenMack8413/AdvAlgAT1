#include "RegexMatcher.hpp"
#include <stack>
#include <cctype>

RegexMatcher::RegexMatcher(const std::string& pattern) {
    pattern_ = pattern;
    std::string formatted = insert_concat_operators(pattern_);
    std::string postfix = infix_to_postfix(formatted);
    build_nfa(postfix);
}

State* RegexMatcher::create_state() {
    auto state = std::make_unique<State>();
    state->id = state_counter_++;
    State* ptr = state.get();
    all_states_.push_back(std::move(state));
    return ptr;
}

// 1. Insert explicit '.' for concatenation (e.g., "ab" -> "a.b")
std::string RegexMatcher::insert_concat_operators(const std::string& pattern) const {
    std::string result;
    for (size_t i = 0; i < pattern.length(); ++i) {
        char c1 = pattern[i];
        result += c1;

        if (i + 1 < pattern.length()) {
            char c2 = pattern[i + 1];
            
            bool c1_can_end = (std::isalnum(c1) || c1 == '*' || c1 == '+' || c1 == '?' || c1 == ')');
            bool c2_can_start = (std::isalnum(c2) || c2 == '(');

            if (c1_can_end && c2_can_start) {
                result += '.'; // Explicit concatenation token
            }
        }
    }
    return result;
}

// 2. Convert Infix expression to Postfix using Dijkstra's Shunting-Yard
std::string RegexMatcher::infix_to_postfix(const std::string& infix) const {
    std::string postfix;
    std::stack<char> op_stack;

    auto precedence = [](char op) {
        if (op == '*' || op == '+' || op == '?') return 3;
        if (op == '.') return 2; // Concatenation
        if (op == '|') return 1; // Alternation
        return 0;
    };

    for (char c : infix) {
        if (std::isalnum(c)) {
            postfix += c;
        } else if (c == '(') {
            op_stack.push(c);
        } else if (c == ')') {
            while (!op_stack.empty() && op_stack.top() != '(') {
                postfix += op_stack.top();
                op_stack.pop();
            }
            if (!op_stack.empty()) op_stack.pop(); // Remove '('
        } else { // Operators: *, +, ?, |, .
            while (!op_stack.empty() && precedence(op_stack.top()) >= precedence(c)) {
                postfix += op_stack.top();
                op_stack.pop();
            }
            op_stack.push(c);
        }
    }

    while (!op_stack.empty()) {
        postfix += op_stack.top();
        op_stack.pop();
    }

    return postfix;
}

// 3. Build Thompson NFA graph from Postfix string
void RegexMatcher::build_nfa(const std::string& postfix) {
    std::stack<Fragment> stack;

    for (char c : postfix) {
        if (std::isalnum(c)) { // Literal character
            State* start = create_state();
            State* accept = create_state();
            start->transitions[c].push_back(accept);
            stack.push({start, accept});
        } 
        else if (c == '.') { // Concatenation (AB)
            Fragment right = stack.top(); stack.pop();
            Fragment left = stack.top(); stack.pop();

            left.accept->epsilon_transitions.push_back(right.start);
            stack.push({left.start, right.accept});
        } 
        else if (c == '|') { // Alternation (A|B)
            Fragment right = stack.top(); stack.pop();
            Fragment left = stack.top(); stack.pop();

            State* start = create_state();
            State* accept = create_state();

            start->epsilon_transitions.push_back(left.start);
            start->epsilon_transitions.push_back(right.start);
            left.accept->epsilon_transitions.push_back(accept);
            right.accept->epsilon_transitions.push_back(accept);

            stack.push({start, accept});
        } 
        else if (c == '*') { // Zero or more (A*)
            Fragment sub = stack.top(); stack.pop();

            State* start = create_state();
            State* accept = create_state();

            start->epsilon_transitions.push_back(sub.start);
            start->epsilon_transitions.push_back(accept);
            sub.accept->epsilon_transitions.push_back(sub.start);
            sub.accept->epsilon_transitions.push_back(accept);

            stack.push({start, accept});
        } 
        else if (c == '+') { // One or more (A+)
            Fragment sub = stack.top(); stack.pop();

            State* start = create_state();
            State* accept = create_state();

            start->epsilon_transitions.push_back(sub.start);
            sub.accept->epsilon_transitions.push_back(sub.start);
            sub.accept->epsilon_transitions.push_back(accept);

            stack.push({start, accept});
        } 
        else if (c == '?') { // Zero or one (A?)
            Fragment sub = stack.top(); stack.pop();

            State* start = create_state();
            State* accept = create_state();

            start->epsilon_transitions.push_back(sub.start);
            start->epsilon_transitions.push_back(accept);
            sub.accept->epsilon_transitions.push_back(accept);

            stack.push({start, accept});
        }
    }

    if (!stack.empty()) {
        Fragment final_nfa = stack.top();
        start_state_ = final_nfa.start;
        accept_state_ = final_nfa.accept;
        accept_state_->is_accept = true;
    }
}

// Helper: Traverses all reachable free ε-transitions recursively
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

// Substring Matcher Execution Engine
MatchResult RegexMatcher::find_match(const std::string& text) const {
    MatchResult result;
    if (!start_state_) return result;

    // Outer loop: Allows searching for matches that start at any character index
    for (size_t start_pos = 0; start_pos < text.length(); ++start_pos) {
        std::unordered_set<State*> current_states = get_epsilon_closure({start_state_});

        for (size_t i = start_pos; i < text.length(); ++i) {
            char c = text[i];
            std::unordered_set<State*> next_states;

            for (State* state : current_states) {
                auto it = state->transitions.find(c);
                if (it != state->transitions.end()) {
                    for (State* target : it->second) {
                        next_states.insert(target);
                    }
                }
            }

            current_states = get_epsilon_closure(next_states);
            
            // Check if we hit an accept state
            for (State* state : current_states) {
                if (state->is_accept) {
                    result.matched = true;
                    result.start_idx = start_pos;
                    result.end_idx = i + 1;
                    return result; // Return first match found
                }
            }

            if (current_states.empty()) break;
        }
    }

    return result;
}
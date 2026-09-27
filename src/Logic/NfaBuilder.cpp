#include "NfaBuilder.hpp"
#include <stack>

NfaGraph NfaBuilder::build_from_postfix(const std::vector<Token>& postfix) {
    NfaGraph graph;
    std::stack<Fragment> stack;
    int state_counter = 0;

    auto create_state = [&graph, &state_counter]() -> State* {
        auto state = std::make_unique<State>();
        state->id = state_counter++;
        State* ptr = state.get();
        graph.all_states.push_back(std::move(state));
        return ptr;
    };

    for (const auto& token : postfix) {
        if (token.type == TokenType::Literal) {
            State* start = create_state();
            State* accept = create_state();
            start->transitions.push_back({token.matcher, accept});
            stack.push({start, accept});
        } 
        else if (token.type == TokenType::Concat) {
            Fragment right = stack.top(); stack.pop();
            Fragment left = stack.top(); stack.pop();

            left.accept->epsilon_transitions.push_back(right.start);
            stack.push({left.start, right.accept});
        } 
        else if (token.type == TokenType::Union) {
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
        else if (token.type == TokenType::Star) {
            Fragment sub = stack.top(); stack.pop();

            State* start = create_state();
            State* accept = create_state();

            start->epsilon_transitions.push_back(sub.start);
            start->epsilon_transitions.push_back(accept);
            sub.accept->epsilon_transitions.push_back(sub.start);
            sub.accept->epsilon_transitions.push_back(accept);

            stack.push({start, accept});
        } 
        else if (token.type == TokenType::Plus) {
            Fragment sub = stack.top(); stack.pop();

            State* start = create_state();
            State* accept = create_state();

            start->epsilon_transitions.push_back(sub.start);
            sub.accept->epsilon_transitions.push_back(sub.start);
            sub.accept->epsilon_transitions.push_back(accept);

            stack.push({start, accept});
        } 
        else if (token.type == TokenType::Question) {
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
        graph.start = final_nfa.start;
        graph.accept = final_nfa.accept;
        graph.accept->is_accept = true;
    }

    return graph;
}
#ifndef NFA_BUILDER_HPP
#define NFA_BUILDER_HPP

#include <vector>
#include <memory>
#include "Token.hpp"

struct NfaGraph {
    State* start = nullptr;
    State* accept = nullptr;
    std::vector<std::unique_ptr<State>> all_states;
};

class NfaBuilder {
public:
    static NfaGraph build_from_postfix(const std::vector<Token>& postfix);
};

#endif // NFA_BUILDER_HPP
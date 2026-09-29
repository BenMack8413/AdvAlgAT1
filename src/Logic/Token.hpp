#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <vector>
#include <functional>
#include <memory>

struct State;

// Flexible transition using a character predicate matcher
struct Transition {
    std::function<bool(char)> matcher;
    State* target = nullptr;
};

// Single NFA Node
struct State {
    int id = 0;
    bool is_accept = false;
    std::vector<Transition> transitions;
    std::vector<State*> epsilon_transitions;
};

enum class TokenType {
    Concat, // 
    Union, // |
    Star, // *
    Plus, // +
    Question, // ?
    OpenParen, // (
    CloseParen, // )
    StartAnchor, // ^
    EndAnchor, // $
    Literal
};

struct Token {
    TokenType type;
    std::function<bool(char)> matcher;
};

// Fragment representing a sub-NFA during construction
struct Fragment {
    State* start = nullptr;
    State* accept = nullptr;
};

#endif // TOKEN_HPP
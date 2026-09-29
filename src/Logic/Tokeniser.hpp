// Tokeniser.hpp
#pragma once
#include "Token.hpp"
#include <string>
#include <vector>

class Tokeniser {
public:
    explicit Tokeniser(const std::string& pattern, bool case_insensitive = false);
    std::vector<Token> tokenise();

private:
    std::string pattern_;
    bool case_insensitive_;
};
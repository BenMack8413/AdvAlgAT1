#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include "Token.hpp"

class Parser {
public:
    static std::vector<Token> insert_concat_operators(const std::vector<Token>& tokens);
    static std::vector<Token> infix_to_postfix(const std::vector<Token>& infix);
};

#endif // PARSER_HPP
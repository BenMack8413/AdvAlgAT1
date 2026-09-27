#ifndef TokeniseR_HPP
#define TokeniseR_HPP

#include <string>
#include <vector>
#include "Token.hpp"

class Tokeniser {
public:
    static std::vector<Token> tokenise(const std::string& pattern);
};

#endif // TokeniseR_HPP
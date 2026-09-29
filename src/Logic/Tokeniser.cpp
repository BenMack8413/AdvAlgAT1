#include "Tokeniser.hpp"
#include <cctype>
#include <stdexcept>

std::vector<Token> Tokeniser::tokenise(const std::string& pattern) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < pattern.length()) {
        char c = pattern[i];

        if (c == '\\') { // Escape sequence handling (\d, \w, \s, \., etc.)
            if (i + 1 >= pattern.length()) {
                throw std::invalid_argument("Trailing backslash in regex pattern");
            }
            char next = pattern[i + 1];
            Token t;
            t.type = TokenType::Literal;

            if (next == 'd') {
                t.matcher = [](char ch) { return std::isdigit(static_cast<unsigned char>(ch)); };
            } else if (next == 'D') {
                t.matcher = [](char ch) { return !std::isdigit(static_cast<unsigned char>(ch)); };
            } else if (next == 'w') {
                t.matcher = [](char ch) { return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_'; };
            } else if (next == 'W') {
                t.matcher = [](char ch) { return !std::isalnum(static_cast<unsigned char>(ch)) && ch != '_'; };
            } else if (next == 's') {
                t.matcher = [](char ch) { return std::isspace(static_cast<unsigned char>(ch)); };
            } else if (next == 'S') {
                t.matcher = [](char ch) { return !std::isspace(static_cast<unsigned char>(ch)); };
            } else if (next == 't') {
                t.matcher = [](char ch) { return ch == '\t'; };
            } else if (next == 'n') {
                t.matcher = [](char ch) { return ch == '\n'; };
            } else {
                t.matcher = [next](char ch) { return ch == next; };
            }
            tokens.push_back(t);
            i += 2;
        } 
        else if (c == '[') { // Bracketed character set/range parsing ([a-z0-9], [^0-9])
            i++;
            bool negated = false;
            if (i < pattern.length() && pattern[i] == '^') {
                negated = true;
                i++;
            }

            std::vector<std::pair<char, char>> ranges;
            std::vector<char> single_chars;

            while (i < pattern.length() && pattern[i] != ']') {
                if (i + 2 < pattern.length() && pattern[i + 1] == '-') {
                    ranges.push_back({pattern[i], pattern[i + 2]});
                    i += 3;
                } else {
                    single_chars.push_back(pattern[i]);
                    i++;
                }
            }
            if (i >= pattern.length()) {
                throw std::invalid_argument("Unclosed character class '['");
            }
            i++; // Consume ']'

            Token t;
            t.type = TokenType::Literal;
            t.matcher = [negated, ranges, single_chars](char ch) {
                bool in_set = false;
                for (char sc : single_chars) {
                    if (ch == sc) { in_set = true; break; }
                }
                if (!in_set) {
                    for (const auto& r : ranges) {
                        if (ch >= r.first && ch <= r.second) { in_set = true; break; }
                    }
                }
                return negated ? !in_set : in_set;
            };
            tokens.push_back(t);
        }
        else if (c == '.') {
            Token t;
            t.type = TokenType::Literal;
            t.matcher = [](char ch) { return ch != '\n'; };
            tokens.push_back(t);
            i++;
        }
        else if (c == '|') { tokens.push_back({TokenType::Union, nullptr}); i++; }
        else if (c == '*') { tokens.push_back({TokenType::Star, nullptr}); i++; }
        else if (c == '+') { tokens.push_back({TokenType::Plus, nullptr}); i++; }
        else if (c == '?') { tokens.push_back({TokenType::Question, nullptr}); i++; }
        else if (c == '(') { tokens.push_back({TokenType::OpenParen, nullptr}); i++; }
        else if (c == ')') { tokens.push_back({TokenType::CloseParen, nullptr}); i++; }
        else {
            Token t;
            t.type = TokenType::Literal;
            t.matcher = [c](char ch) { return ch == c; };
            tokens.push_back(t);
            i++;
        }
    }
    return tokens;
}
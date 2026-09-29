#include "Tokeniser.hpp"
#include <cctype>
#include <stdexcept>
#include <cctype>

Tokeniser::Tokeniser(const std::string& pattern, bool case_insensitive) 
    : pattern_(pattern), case_insensitive_(case_insensitive) {}

std::vector<Token> Tokeniser::tokenise() {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < pattern_.length()) {
        char c = pattern_[i];

        if (c == '\\') { // Escape sequence handling (\d, \w, \s, \., etc.)
            if (i + 1 >= pattern_.length()) {
                throw std::invalid_argument("Trailing backslash in regex pattern");
            }
            char next = pattern_[i + 1];
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
                // Escaped literal character (e.g. '\a', '\.', '\-')
                bool ci = case_insensitive_;
                t.matcher = [next, ci](char ch) {
                    if (ci) {
                        return std::tolower(static_cast<unsigned char>(ch)) == 
                               std::tolower(static_cast<unsigned char>(next));
                    }
                    return ch == next;
                };
            }
            tokens.push_back(t);
            i += 2;
        } 
        else if (c == '[') { // Bracketed character set/range parsing ([a-z0-9], [^0-9])
            i++;
            bool negated = false;
            if (i < pattern_.length() && pattern_[i] == '^') {
                negated = true;
                i++;
            }

            std::vector<std::pair<char, char>> ranges;
            std::vector<char> single_chars;

            while (i < pattern_.length() && pattern_[i] != ']') {
                if (i + 2 < pattern_.length() && pattern_[i + 1] == '-') {
                    ranges.push_back({pattern_[i], pattern_[i + 2]});
                    i += 3;
                } else {
                    single_chars.push_back(pattern_[i]);
                    i++;
                }
            }
            if (i >= pattern_.length()) {
                throw std::invalid_argument("Unclosed character class '['");
            }
            i++; // Consume ']'

            Token t;
            t.type = TokenType::Literal;
            bool ci = case_insensitive_;
            
            t.matcher = [negated, ranges, single_chars, ci](char ch) {
                unsigned char input_c = ci ? std::tolower(static_cast<unsigned char>(ch))
                                           : static_cast<unsigned char>(ch);
                bool in_set = false;

                // 1. Check single characters in class
                for (char sc : single_chars) {
                    unsigned char target = ci ? std::tolower(static_cast<unsigned char>(sc))
                                             : static_cast<unsigned char>(sc);
                    if (input_c == target) {
                        in_set = true;
                        break;
                    }
                }

                // 2. Check ranges in class
                if (!in_set) {
                    for (const auto& r : ranges) {
                        unsigned char r1 = ci ? std::tolower(static_cast<unsigned char>(r.first))
                                              : static_cast<unsigned char>(r.first);
                        unsigned char r2 = ci ? std::tolower(static_cast<unsigned char>(r.second))
                                              : static_cast<unsigned char>(r.second);
                        
                        unsigned char r_min = std::min(r1, r2);
                        unsigned char r_max = std::max(r1, r2);

                        if (input_c >= r_min && input_c <= r_max) {
                            in_set = true;
                            break;
                        }
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
        else if (c == '^') { tokens.push_back({TokenType::StartAnchor, nullptr}); i++; }
        else if (c == '$') { tokens.push_back({TokenType::EndAnchor, nullptr}); i++; }
        else if (c == '|') { tokens.push_back({TokenType::Union, nullptr}); i++; }
        else if (c == '*') { tokens.push_back({TokenType::Star, nullptr}); i++; }
        else if (c == '+') { tokens.push_back({TokenType::Plus, nullptr}); i++; }
        else if (c == '?') { tokens.push_back({TokenType::Question, nullptr}); i++; }
        else if (c == '(') { tokens.push_back({TokenType::OpenParen, nullptr}); i++; }
        else if (c == ')') { tokens.push_back({TokenType::CloseParen, nullptr}); i++; }
        else {
            // Unescaped plain literal character
            Token t;
            t.type = TokenType::Literal;
            bool ci = case_insensitive_;
            
            t.matcher = [c, ci](char ch) {
                if (ci) {
                    return std::tolower(static_cast<unsigned char>(ch)) == 
                           std::tolower(static_cast<unsigned char>(c));
                }
                return ch == c;
            };
            tokens.push_back(t);
            i++;
        }
    }
    return tokens;
}
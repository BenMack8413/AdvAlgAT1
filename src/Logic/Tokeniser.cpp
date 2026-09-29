#include "Tokeniser.hpp"
#include <cctype>
#include <stdexcept>

Tokeniser::Tokeniser(const std::string& pattern, bool case_insensitive) 
    : pattern_(pattern), case_insensitive_(case_insensitive) {}

std::vector<Token> Tokeniser::tokenise() {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < pattern_.length()) {
        char c = pattern_[i];

        if (c == '\\') {
            if (i + 1 >= pattern_.length()) {
                throw std::invalid_argument("Trailing backslash in regex pattern");
            }
            char next = pattern_[i + 1];

            if (next == 'b') {
                tokens.push_back({TokenType::WordBoundary, nullptr});
                i += 2;
                continue;
            } else if (next == 'B') {
                tokens.push_back({TokenType::NonWordBoundary, nullptr});
                i += 2;
                continue;
            }
            if (next == 'x' && i + 3 < pattern_.length()) {
                std::string hex_str = pattern_.substr(i + 2, 2);
                char hex_val = static_cast<char>(std::stoul(hex_str, nullptr, 16));
                
                Token t;
                t.type = TokenType::Literal;
                bool ci = case_insensitive_;
                t.matcher = [hex_val, ci](char ch) {
                    if (ci) return std::tolower(static_cast<unsigned char>(ch)) == std::tolower(static_cast<unsigned char>(hex_val));
                    return ch == hex_val;
                };
                tokens.push_back(t);
                i += 4;
                continue;
            }

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
        else if (c == '[') {
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

                for (char sc : single_chars) {
                    unsigned char target = ci ? std::tolower(static_cast<unsigned char>(sc))
                                             : static_cast<unsigned char>(sc);
                    if (input_c == target) {
                        in_set = true;
                        break;
                    }
                }

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
        else if (c == '*') {
            if (i + 1 < pattern_.length() && pattern_[i + 1] == '?') {
                tokens.push_back({TokenType::LazyStar, nullptr}); i += 2;
            } else {
                tokens.push_back({TokenType::Star, nullptr}); i++;
            }
        }
        else if (c == '+') {
            if (i + 1 < pattern_.length() && pattern_[i + 1] == '?') {
                tokens.push_back({TokenType::LazyPlus, nullptr}); i += 2;
            } else {
                tokens.push_back({TokenType::Plus, nullptr}); i++;
            }
        }
        else if (c == '?') {
            if (i + 1 < pattern_.length() && pattern_[i + 1] == '?') {
                tokens.push_back({TokenType::LazyQuestion, nullptr}); i += 2;
            } else {
                tokens.push_back({TokenType::Question, nullptr}); i++;
            }
        }
        else if (c == '(') { tokens.push_back({TokenType::OpenParen, nullptr}); i++; }
        else if (c == ')') { tokens.push_back({TokenType::CloseParen, nullptr}); i++; }
        else if (c == '{') {
            size_t j = i + 1;
            bool valid = false;
            size_t n = 0;
            int m = -1; // -1 represents unbounded {n,}

            if (j < pattern_.length() && std::isdigit(static_cast<unsigned char>(pattern_[j]))) {
                while (j < pattern_.length() && std::isdigit(static_cast<unsigned char>(pattern_[j]))) {
                    n = n * 10 + (pattern_[j] - '0');
                    j++;
                }

                if (j < pattern_.length()) {
                    if (pattern_[j] == '}') {
                        m = static_cast<int>(n);
                        valid = true;
                        j++;
                    } else if (pattern_[j] == ',') {
                        j++;
                        if (j < pattern_.length() && pattern_[j] == '}') {
                            m = -1;
                            valid = true;
                            j++;
                        } else if (j < pattern_.length() && std::isdigit(static_cast<unsigned char>(pattern_[j]))) {
                            size_t parsed_m = 0;
                            while (j < pattern_.length() && std::isdigit(static_cast<unsigned char>(pattern_[j]))) {
                                parsed_m = parsed_m * 10 + (pattern_[j] - '0');
                                j++;
                            }
                            if (j < pattern_.length() && pattern_[j] == '}' && parsed_m >= n) {
                                m = static_cast<int>(parsed_m);
                                valid = true;
                                j++;
                            }
                        }
                    }
                }
            }

            int atom_start = -1;
            if (valid && !tokens.empty()) {
                TokenType last_type = tokens.back().type;
                if (last_type == TokenType::CloseParen) {
                    int depth = 0;
                    for (int k = static_cast<int>(tokens.size()) - 1; k >= 0; --k) {
                        if (tokens[k].type == TokenType::CloseParen) {
                            depth++;
                        } else if (tokens[k].type == TokenType::OpenParen) {
                            depth--;
                            if (depth == 0) {
                                atom_start = k;
                                break;
                            }
                        }
                    }
                } else if (last_type == TokenType::Literal || 
                           last_type == TokenType::WordBoundary || 
                           last_type == TokenType::NonWordBoundary) {
                    atom_start = static_cast<int>(tokens.size()) - 1;
                }
            }

            if (valid && atom_start != -1) {
                bool lazy = false;
                if (j < pattern_.length() && pattern_[j] == '?') {
                    lazy = true;
                    j++;
                }
                i = j;
                std::vector<Token> atom(tokens.begin() + atom_start, tokens.end());
                tokens.erase(tokens.begin() + atom_start, tokens.end());

                if (m == -1) {
                    if (n == 0) {
                        tokens.insert(tokens.end(), atom.begin(), atom.end());
                        tokens.push_back({lazy ? TokenType::LazyStar : TokenType::Star, nullptr});
                    } else if (n == 1) {
                        tokens.insert(tokens.end(), atom.begin(), atom.end());
                        tokens.push_back({lazy ? TokenType::LazyPlus : TokenType::Plus, nullptr});
                    } else {
                        for (size_t rep = 0; rep < n - 1; ++rep) {
                            tokens.insert(tokens.end(), atom.begin(), atom.end());
                        }
                        tokens.insert(tokens.end(), atom.begin(), atom.end());
                        tokens.push_back({lazy ? TokenType::LazyPlus : TokenType::Plus, nullptr});
                    }
                } else {
                    for (size_t rep = 0; rep < n; ++rep) {
                        tokens.insert(tokens.end(), atom.begin(), atom.end());
                    }
                    for (size_t rep = 0; rep < static_cast<size_t>(m) - n; ++rep) {
                        tokens.insert(tokens.end(), atom.begin(), atom.end());
                        tokens.push_back({lazy ? TokenType::LazyQuestion : TokenType::Question, nullptr});
                    }
                }
            } else {
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
        else {
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
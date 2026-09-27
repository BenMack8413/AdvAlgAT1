#include "Parser.hpp"
#include <stack>

std::vector<Token> Parser::insert_concat_operators(const std::vector<Token>& tokens) {
    std::vector<Token> result;

    auto can_end = [](TokenType type) {
        return type == TokenType::Literal || type == TokenType::Star || 
               type == TokenType::Plus || type == TokenType::Question || 
               type == TokenType::CloseParen;
    };

    auto can_start = [](TokenType type) {
        return type == TokenType::Literal || type == TokenType::OpenParen;
    };

    for (size_t i = 0; i < tokens.size(); ++i) {
        result.push_back(tokens[i]);

        if (i + 1 < tokens.size()) {
            if (can_end(tokens[i].type) && can_start(tokens[i + 1].type)) {
                result.push_back({TokenType::Concat, nullptr});
            }
        }
    }
    return result;
}

std::vector<Token> Parser::infix_to_postfix(const std::vector<Token>& infix) {
    std::vector<Token> postfix;
    std::stack<Token> op_stack;

    auto precedence = [](TokenType type) {
        if (type == TokenType::Star || type == TokenType::Plus || type == TokenType::Question) return 3;
        if (type == TokenType::Concat) return 2;
        if (type == TokenType::Union) return 1;
        return 0;
    };

    for (const auto& token : infix) {
        if (token.type == TokenType::Literal) {
            postfix.push_back(token);
        } else if (token.type == TokenType::OpenParen) {
            op_stack.push(token);
        } else if (token.type == TokenType::CloseParen) {
            while (!op_stack.empty() && op_stack.top().type != TokenType::OpenParen) {
                postfix.push_back(op_stack.top());
                op_stack.pop();
            }
            if (!op_stack.empty()) op_stack.pop();
        } else {
            while (!op_stack.empty() && precedence(op_stack.top().type) >= precedence(token.type)) {
                postfix.push_back(op_stack.top());
                op_stack.pop();
            }
            op_stack.push(token);
        }
    }

    while (!op_stack.empty()) {
        postfix.push_back(op_stack.top());
        op_stack.pop();
    }

    return postfix;
}
#pragma once

#include <string>
#include <vector>
#include <stack>

#include "Token.hpp"

namespace ink
{

    class Lexer
    {
    public:
        explicit Lexer(const std::string &source);
        std::vector<Token> tokenize();

    private:
        char peek() const;
        char advance();
        bool isAtEnd() const;
        bool match(char expected);

        Token makeToken(TokenType type, const std::string &value = "");
        Token readNumber();
        Token readIdentifier();
        void processIndentation();

        std::string m_source;
        size_t m_pos{0};
        int m_line{1};
        int m_col{1};
        bool m_atLineStart{true};
        std::stack<int> m_indentStack;
        std::vector<Token> m_tokens;
    };

} // namespace ink

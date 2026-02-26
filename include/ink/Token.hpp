#pragma once

#include <string>
#include <cstdint>

namespace ink
{

    enum class TokenType : uint8_t
    {
        // Literals
        NUMBER,
        IDENTIFIER,

        // Keywords
        IF,
        ELIF,
        ELSE,
        OR,
        AND,
        NOT,
        BEHAVIOR, // @behavior

        // Arithmetic
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PERCENT,

        // Comparison
        LT,
        GT,
        LTE,
        GTE,
        EQ_EQ,
        BANG_EQ,

        // Assignment
        ASSIGN,
        PLUS_EQ,
        MINUS_EQ,
        STAR_EQ,
        SLASH_EQ,

        // Delimiters
        LPAREN,
        RPAREN,
        COLON,
        DOT,
        COMMA,

        // Structure
        NEWLINE,
        INDENT,
        DEDENT,
        END_OF_FILE,
    };

    struct Token
    {
        TokenType type;
        std::string value;
        int line{0};
        int col{0};
    };

} // namespace ink

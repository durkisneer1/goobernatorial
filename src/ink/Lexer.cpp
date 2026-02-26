#include "ink/Lexer.hpp"

#include <stdexcept>
#include <cctype>

namespace ink
{

    Lexer::Lexer(const std::string &source)
        : m_source(source)
    {
        m_indentStack.push(0);
    }

    char Lexer::peek() const
    {
        if (isAtEnd())
            return '\0';
        return m_source[m_pos];
    }

    char Lexer::advance()
    {
        char c = m_source[m_pos++];
        if (c == '\n')
        {
            m_line++;
            m_col = 1;
        }
        else
        {
            m_col++;
        }
        return c;
    }

    bool Lexer::isAtEnd() const
    {
        return m_pos >= m_source.size();
    }

    bool Lexer::match(char expected)
    {
        if (isAtEnd() || m_source[m_pos] != expected)
            return false;
        advance();
        return true;
    }

    Token Lexer::makeToken(TokenType type, const std::string &value)
    {
        return Token{type, value, m_line, m_col};
    }

    Token Lexer::readNumber()
    {
        size_t start = m_pos;
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
            advance();
        if (!isAtEnd() && peek() == '.')
        {
            advance();
            while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
                advance();
        }
        return makeToken(TokenType::NUMBER, m_source.substr(start, m_pos - start));
    }

    Token Lexer::readIdentifier()
    {
        size_t start = m_pos;
        while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_'))
            advance();
        std::string word = m_source.substr(start, m_pos - start);

        // Check keywords
        if (word == "if")
            return makeToken(TokenType::IF, word);
        if (word == "elif")
            return makeToken(TokenType::ELIF, word);
        if (word == "else")
            return makeToken(TokenType::ELSE, word);
        if (word == "or")
            return makeToken(TokenType::OR, word);
        if (word == "and")
            return makeToken(TokenType::AND, word);
        if (word == "not")
            return makeToken(TokenType::NOT, word);

        return makeToken(TokenType::IDENTIFIER, word);
    }

    void Lexer::processIndentation()
    {
        int indent = 0;
        while (!isAtEnd())
        {
            if (peek() == ' ')
            {
                indent++;
                advance();
            }
            else if (peek() == '\t')
            {
                indent += 4;
                advance();
            }
            else
            {
                break;
            }
        }

        // Skip blank lines and comment-only lines — don't change indentation
        if (isAtEnd() || peek() == '\n' || peek() == '#')
        {
            return;
        }

        int currentIndent = m_indentStack.top();

        if (indent > currentIndent)
        {
            m_indentStack.push(indent);
            m_tokens.push_back(makeToken(TokenType::INDENT));
        }
        else
        {
            while (indent < m_indentStack.top())
            {
                m_indentStack.pop();
                m_tokens.push_back(makeToken(TokenType::DEDENT));
            }
            if (indent != m_indentStack.top())
            {
                throw std::runtime_error(
                    "Ink: inconsistent indentation at line " + std::to_string(m_line));
            }
        }

        m_atLineStart = false;
    }

    std::vector<Token> Lexer::tokenize()
    {
        while (!isAtEnd())
        {
            // At line start, process indentation
            if (m_atLineStart)
            {
                processIndentation();
                if (isAtEnd())
                    break;

                // Skip blank lines
                if (peek() == '\n')
                {
                    advance();
                    continue;
                }
                // Skip comment lines
                if (peek() == '#')
                {
                    while (!isAtEnd() && peek() != '\n')
                        advance();
                    if (!isAtEnd())
                        advance();
                    m_atLineStart = true;
                    continue;
                }
            }

            char c = peek();

            // Skip inline whitespace
            if (c == ' ' || c == '\t')
            {
                advance();
                continue;
            }

            // Carriage return
            if (c == '\r')
            {
                advance();
                continue;
            }

            // Newline
            if (c == '\n')
            {
                advance();
                if (!m_tokens.empty() &&
                    m_tokens.back().type != TokenType::NEWLINE &&
                    m_tokens.back().type != TokenType::INDENT)
                {
                    m_tokens.push_back(makeToken(TokenType::NEWLINE));
                }
                m_atLineStart = true;
                continue;
            }

            // Comment
            if (c == '#')
            {
                while (!isAtEnd() && peek() != '\n')
                    advance();
                continue;
            }

            // @behavior directive
            if (c == '@')
            {
                advance();
                Token ident = readIdentifier();
                if (ident.value == "behavior")
                {
                    m_tokens.push_back(makeToken(TokenType::BEHAVIOR, "behavior"));
                }
                else
                {
                    throw std::runtime_error(
                        "Ink: unknown directive @" + ident.value +
                        " at line " + std::to_string(m_line));
                }
                continue;
            }

            // Number
            if (std::isdigit(static_cast<unsigned char>(c)))
            {
                m_tokens.push_back(readNumber());
                continue;
            }

            // Identifier / keyword
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
            {
                m_tokens.push_back(readIdentifier());
                continue;
            }

            // Operators and delimiters
            advance();
            switch (c)
            {
            case '+':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::PLUS_EQ, "+=")
                                       : makeToken(TokenType::PLUS, "+"));
                break;
            case '-':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::MINUS_EQ, "-=")
                                       : makeToken(TokenType::MINUS, "-"));
                break;
            case '*':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::STAR_EQ, "*=")
                                       : makeToken(TokenType::STAR, "*"));
                break;
            case '/':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::SLASH_EQ, "/=")
                                       : makeToken(TokenType::SLASH, "/"));
                break;
            case '%':
                m_tokens.push_back(makeToken(TokenType::PERCENT, "%"));
                break;
            case '<':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::LTE, "<=")
                                       : makeToken(TokenType::LT, "<"));
                break;
            case '>':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::GTE, ">=")
                                       : makeToken(TokenType::GT, ">"));
                break;
            case '=':
                m_tokens.push_back(match('=')
                                       ? makeToken(TokenType::EQ_EQ, "==")
                                       : makeToken(TokenType::ASSIGN, "="));
                break;
            case '!':
                if (match('='))
                {
                    m_tokens.push_back(makeToken(TokenType::BANG_EQ, "!="));
                }
                else
                {
                    throw std::runtime_error(
                        "Ink: unexpected '!' at line " + std::to_string(m_line) +
                        ". Did you mean '!='?");
                }
                break;
            case '(':
                m_tokens.push_back(makeToken(TokenType::LPAREN, "("));
                break;
            case ')':
                m_tokens.push_back(makeToken(TokenType::RPAREN, ")"));
                break;
            case ':':
                m_tokens.push_back(makeToken(TokenType::COLON, ":"));
                break;
            case '.':
                m_tokens.push_back(makeToken(TokenType::DOT, "."));
                break;
            case ',':
                m_tokens.push_back(makeToken(TokenType::COMMA, ","));
                break;
            default:
                throw std::runtime_error(
                    "Ink: unexpected character '" + std::string(1, c) +
                    "' at line " + std::to_string(m_line));
            }
        }

        // Final newline if needed
        if (!m_tokens.empty() && m_tokens.back().type != TokenType::NEWLINE)
        {
            m_tokens.push_back(makeToken(TokenType::NEWLINE));
        }

        // Close all open indentation levels
        while (m_indentStack.size() > 1)
        {
            m_indentStack.pop();
            m_tokens.push_back(makeToken(TokenType::DEDENT));
        }

        m_tokens.push_back(makeToken(TokenType::END_OF_FILE));
        return m_tokens;
    }

} // namespace ink

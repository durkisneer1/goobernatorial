#pragma once

#include <vector>
#include <stdexcept>

#include "Token.hpp"
#include "AST.hpp"

namespace ink
{

    class ParseError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class Parser
    {
    public:
        explicit Parser(const std::vector<Token> &tokens);
        BehaviorDecl parse();

    private:
        // Token navigation
        const Token &peek() const;
        const Token &previous() const;
        const Token &advance();
        bool check(TokenType type) const;
        bool match(TokenType type);
        const Token &expect(TokenType type, const std::string &msg);
        bool isAtEnd() const;
        void skipNewlines();

        // Grammar rules
        BehaviorDecl parseBehavior();
        std::unique_ptr<Block> parseBlock();
        StmtPtr parseStatement();
        StmtPtr parseIfStatement();
        StmtPtr parseAssignmentOrExpr();

        // Expressions (precedence climbing)
        ExprPtr parseExpression();
        ExprPtr parseOr();
        ExprPtr parseAnd();
        ExprPtr parseNot();
        ExprPtr parseComparison();
        ExprPtr parseAddSub();
        ExprPtr parseMulDivMod();
        ExprPtr parseUnary();
        ExprPtr parsePrimary();
        ExprPtr parseFieldOrIdent();

        const std::vector<Token> &m_tokens;
        size_t m_pos{0};
    };

} // namespace ink

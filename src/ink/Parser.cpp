#include "ink/Parser.hpp"

namespace ink
{

    Parser::Parser(const std::vector<Token> &tokens)
        : m_tokens(tokens) {}

    const Token &Parser::peek() const
    {
        return m_tokens[m_pos];
    }

    const Token &Parser::previous() const
    {
        return m_tokens[m_pos - 1];
    }

    const Token &Parser::advance()
    {
        if (!isAtEnd())
            m_pos++;
        return previous();
    }

    bool Parser::check(TokenType type) const
    {
        if (isAtEnd())
            return false;
        return peek().type == type;
    }

    bool Parser::match(TokenType type)
    {
        if (check(type))
        {
            advance();
            return true;
        }
        return false;
    }

    const Token &Parser::expect(TokenType type, const std::string &msg)
    {
        if (check(type))
            return advance();
        throw ParseError(
            "Ink parse error (line " + std::to_string(peek().line) +
            "): expected " + msg + ", got '" + peek().value + "'");
    }

    bool Parser::isAtEnd() const
    {
        return m_pos >= m_tokens.size() || peek().type == TokenType::END_OF_FILE;
    }

    void Parser::skipNewlines()
    {
        while (check(TokenType::NEWLINE))
            advance();
    }

    // ======================== Top-level ========================

    BehaviorDecl Parser::parse()
    {
        skipNewlines();
        auto behavior = parseBehavior();
        skipNewlines();
        if (!isAtEnd())
        {
            throw ParseError(
                "Ink parse error (line " + std::to_string(peek().line) +
                "): unexpected token after behavior block: '" + peek().value + "'");
        }
        return behavior;
    }

    BehaviorDecl Parser::parseBehavior()
    {
        expect(TokenType::BEHAVIOR, "@behavior");
        auto nameToken = expect(TokenType::IDENTIFIER, "behavior name");
        expect(TokenType::COLON, "':'");
        expect(TokenType::NEWLINE, "newline after ':'");

        BehaviorDecl decl;
        decl.name = nameToken.value;
        decl.body = parseBlock();
        return decl;
    }

    // ======================== Blocks & Statements ========================

    std::unique_ptr<Block> Parser::parseBlock()
    {
        expect(TokenType::INDENT, "indented block");

        auto block = std::make_unique<Block>();

        while (!check(TokenType::DEDENT) && !isAtEnd())
        {
            skipNewlines();
            if (check(TokenType::DEDENT) || isAtEnd())
                break;
            block->stmts.push_back(parseStatement());
        }

        if (check(TokenType::DEDENT))
            advance();

        return block;
    }

    StmtPtr Parser::parseStatement()
    {
        if (check(TokenType::IF))
            return parseIfStatement();
        return parseAssignmentOrExpr();
    }

    StmtPtr Parser::parseIfStatement()
    {
        auto ifStmt = std::make_unique<IfStmt>();

        // 'if' branch
        expect(TokenType::IF, "'if'");
        auto cond = parseExpression();
        expect(TokenType::COLON, "':'");
        expect(TokenType::NEWLINE, "newline");
        auto body = parseBlock();

        IfBranch branch;
        branch.condition = std::move(cond);
        branch.body = std::move(body);
        ifStmt->branches.push_back(std::move(branch));

        // 'elif' branches
        skipNewlines();
        while (check(TokenType::ELIF))
        {
            advance();
            auto elifCond = parseExpression();
            expect(TokenType::COLON, "':'");
            expect(TokenType::NEWLINE, "newline");
            auto elifBody = parseBlock();

            IfBranch elifBranch;
            elifBranch.condition = std::move(elifCond);
            elifBranch.body = std::move(elifBody);
            ifStmt->branches.push_back(std::move(elifBranch));
            skipNewlines();
        }

        // optional 'else' branch
        if (check(TokenType::ELSE))
        {
            advance();
            expect(TokenType::COLON, "':'");
            expect(TokenType::NEWLINE, "newline");
            ifStmt->elseBranch = parseBlock();
        }

        return ifStmt;
    }

    StmtPtr Parser::parseAssignmentOrExpr()
    {
        // Expect: field (= | += | -= | *= | /=) expression NEWLINE
        if (check(TokenType::IDENTIFIER))
        {
            size_t savedPos = m_pos;

            std::string name = advance().value;

            // Check for dot access (pos.x, dir.y, etc.)
            if (match(TokenType::DOT))
            {
                auto fieldToken = expect(TokenType::IDENTIFIER, "field name after '.'");
                name = name + "." + fieldToken.value;
            }

            // Simple assignment: field = expr
            if (check(TokenType::ASSIGN))
            {
                advance();
                auto value = parseExpression();
                expect(TokenType::NEWLINE, "newline");
                return std::make_unique<AssignStmt>(std::move(name), std::move(value));
            }

            // Compound assignment: field += expr, etc.
            if (check(TokenType::PLUS_EQ) || check(TokenType::MINUS_EQ) ||
                check(TokenType::STAR_EQ) || check(TokenType::SLASH_EQ))
            {
                CompoundOp op;
                switch (peek().type)
                {
                case TokenType::PLUS_EQ:
                    op = CompoundOp::ADD_EQ;
                    break;
                case TokenType::MINUS_EQ:
                    op = CompoundOp::SUB_EQ;
                    break;
                case TokenType::STAR_EQ:
                    op = CompoundOp::MUL_EQ;
                    break;
                case TokenType::SLASH_EQ:
                    op = CompoundOp::DIV_EQ;
                    break;
                default:
                    throw ParseError("Unexpected compound operator");
                }
                advance();
                auto value = parseExpression();
                expect(TokenType::NEWLINE, "newline");
                return std::make_unique<CompoundAssignStmt>(
                    std::move(name), op, std::move(value));
            }

            // Not an assignment — backtrack
            m_pos = savedPos;
        }

        throw ParseError(
            "Ink parse error (line " + std::to_string(peek().line) +
            "): expected assignment statement, got '" + peek().value + "'");
    }

    // ======================== Expressions ========================

    ExprPtr Parser::parseExpression()
    {
        return parseOr();
    }

    ExprPtr Parser::parseOr()
    {
        auto left = parseAnd();
        while (match(TokenType::OR))
        {
            auto right = parseAnd();
            left = std::make_unique<BinaryExpr>(BinOp::OR, std::move(left), std::move(right));
        }
        return left;
    }

    ExprPtr Parser::parseAnd()
    {
        auto left = parseNot();
        while (match(TokenType::AND))
        {
            auto right = parseNot();
            left = std::make_unique<BinaryExpr>(BinOp::AND, std::move(left), std::move(right));
        }
        return left;
    }

    ExprPtr Parser::parseNot()
    {
        if (match(TokenType::NOT))
        {
            auto operand = parseNot();
            return std::make_unique<UnaryExpr>(UnaryOp::NOT, std::move(operand));
        }
        return parseComparison();
    }

    ExprPtr Parser::parseComparison()
    {
        auto left = parseAddSub();

        if (check(TokenType::LT) || check(TokenType::GT) ||
            check(TokenType::LTE) || check(TokenType::GTE) ||
            check(TokenType::EQ_EQ) || check(TokenType::BANG_EQ))
        {
            BinOp op;
            switch (peek().type)
            {
            case TokenType::LT:
                op = BinOp::LT;
                break;
            case TokenType::GT:
                op = BinOp::GT;
                break;
            case TokenType::LTE:
                op = BinOp::LTE;
                break;
            case TokenType::GTE:
                op = BinOp::GTE;
                break;
            case TokenType::EQ_EQ:
                op = BinOp::EQ;
                break;
            case TokenType::BANG_EQ:
                op = BinOp::NEQ;
                break;
            default:
                break;
            }
            advance();
            auto right = parseAddSub();
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }

        return left;
    }

    ExprPtr Parser::parseAddSub()
    {
        auto left = parseMulDivMod();

        while (check(TokenType::PLUS) || check(TokenType::MINUS))
        {
            BinOp op = (peek().type == TokenType::PLUS) ? BinOp::ADD : BinOp::SUB;
            advance();
            auto right = parseMulDivMod();
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }

        return left;
    }

    ExprPtr Parser::parseMulDivMod()
    {
        auto left = parseUnary();

        while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT))
        {
            BinOp op;
            switch (peek().type)
            {
            case TokenType::STAR:
                op = BinOp::MUL;
                break;
            case TokenType::SLASH:
                op = BinOp::DIV;
                break;
            case TokenType::PERCENT:
                op = BinOp::MOD;
                break;
            default:
                break;
            }
            advance();
            auto right = parseUnary();
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }

        return left;
    }

    ExprPtr Parser::parseUnary()
    {
        if (match(TokenType::MINUS))
        {
            auto operand = parseUnary();
            return std::make_unique<UnaryExpr>(UnaryOp::NEG, std::move(operand));
        }
        return parsePrimary();
    }

    ExprPtr Parser::parsePrimary()
    {
        // Number literal
        if (check(TokenType::NUMBER))
        {
            double val = std::stod(advance().value);
            return std::make_unique<NumberLiteral>(val);
        }

        // Identifier (possibly dotted field access)
        if (check(TokenType::IDENTIFIER))
        {
            return parseFieldOrIdent();
        }

        // Parenthesized expression
        if (match(TokenType::LPAREN))
        {
            auto expr = parseExpression();
            expect(TokenType::RPAREN, "')'");
            return expr;
        }

        throw ParseError(
            "Ink parse error (line " + std::to_string(peek().line) +
            "): expected expression, got '" + peek().value + "'");
    }

    ExprPtr Parser::parseFieldOrIdent()
    {
        std::string name = advance().value;

        if (match(TokenType::DOT))
        {
            auto fieldToken = expect(TokenType::IDENTIFIER, "field name after '.'");
            return std::make_unique<FieldAccess>(name, fieldToken.value);
        }

        return std::make_unique<FieldAccess>(name);
    }

} // namespace ink

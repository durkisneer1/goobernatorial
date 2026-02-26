#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace ink
{

    // ======================== Expressions ========================

    enum class ExprKind : uint8_t
    {
        NUMBER,
        FIELD,
        BINARY,
        UNARY,
    };

    enum class BinOp : uint8_t
    {
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        LT,
        GT,
        LTE,
        GTE,
        EQ,
        NEQ,
        AND,
        OR,
    };

    enum class UnaryOp : uint8_t
    {
        NEG,
        NOT,
    };

    enum class CompoundOp : uint8_t
    {
        ADD_EQ,
        SUB_EQ,
        MUL_EQ,
        DIV_EQ,
    };

    struct Expr
    {
        const ExprKind kind;
        virtual ~Expr() = default;

    protected:
        explicit Expr(ExprKind k) : kind(k) {}
    };

    using ExprPtr = std::unique_ptr<Expr>;

    struct NumberLiteral : Expr
    {
        double value;
        explicit NumberLiteral(double v) : Expr(ExprKind::NUMBER), value(v) {}
    };

    struct FieldAccess : Expr
    {
        std::string object; // e.g. "pos", "dir", "" for simple fields like "rot"
        std::string field;  // e.g. "x", "y", or the field name itself

        std::string fullName() const
        {
            return object.empty() ? field : object + "." + field;
        }

        FieldAccess(const std::string &obj, const std::string &f)
            : Expr(ExprKind::FIELD), object(obj), field(f) {}
        explicit FieldAccess(const std::string &f)
            : Expr(ExprKind::FIELD), field(f) {}
    };

    struct BinaryExpr : Expr
    {
        BinOp op;
        ExprPtr left, right;
        BinaryExpr(BinOp o, ExprPtr l, ExprPtr r)
            : Expr(ExprKind::BINARY), op(o), left(std::move(l)), right(std::move(r)) {}
    };

    struct UnaryExpr : Expr
    {
        UnaryOp op;
        ExprPtr operand;
        UnaryExpr(UnaryOp o, ExprPtr e)
            : Expr(ExprKind::UNARY), op(o), operand(std::move(e)) {}
    };

    // ======================== Statements ========================

    enum class StmtKind : uint8_t
    {
        IF,
        ASSIGN,
        COMPOUND_ASSIGN,
    };

    struct Stmt
    {
        const StmtKind kind;
        virtual ~Stmt() = default;

    protected:
        explicit Stmt(StmtKind k) : kind(k) {}
    };

    using StmtPtr = std::unique_ptr<Stmt>;

    struct Block
    {
        std::vector<StmtPtr> stmts;
    };

    struct AssignStmt : Stmt
    {
        std::string target;
        ExprPtr value;
        AssignStmt(std::string t, ExprPtr v)
            : Stmt(StmtKind::ASSIGN), target(std::move(t)), value(std::move(v)) {}
    };

    struct CompoundAssignStmt : Stmt
    {
        std::string target;
        CompoundOp op;
        ExprPtr value;
        CompoundAssignStmt(std::string t, CompoundOp o, ExprPtr v)
            : Stmt(StmtKind::COMPOUND_ASSIGN), target(std::move(t)), op(o), value(std::move(v)) {}
    };

    struct IfBranch
    {
        ExprPtr condition;
        std::unique_ptr<Block> body;
    };

    struct IfStmt : Stmt
    {
        std::vector<IfBranch> branches; // if + elifs
        std::unique_ptr<Block> elseBranch;
        IfStmt() : Stmt(StmtKind::IF) {}
    };

    // ======================== Top-level ========================

    struct BehaviorDecl
    {
        std::string name;
        std::unique_ptr<Block> body;
    };

} // namespace ink

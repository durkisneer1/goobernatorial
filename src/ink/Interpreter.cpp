#include "ink/Interpreter.hpp"

#include <cmath>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ink
{

    void Interpreter::bindField(const std::string &name, double *data)
    {
        m_fields[name] = data;
    }

    void Interpreter::setConstant(const std::string &name, double value)
    {
        m_constants[name] = value;
    }

    void Interpreter::setCount(size_t count)
    {
        m_count = count;
    }

    void Interpreter::execute(const BehaviorDecl &behavior)
    {
        if (m_count == 0 || !behavior.body)
            return;
        m_activeMask.assign(m_count, 1.0);
        m_maskStack.clear();
        execBlock(*behavior.body);
    }

    // ======================== Helpers ========================

    static inline double applyBinOp(BinOp op, double l, double r)
    {
        switch (op)
        {
        case BinOp::ADD:
            return l + r;
        case BinOp::SUB:
            return l - r;
        case BinOp::MUL:
            return l * r;
        case BinOp::DIV:
            return r != 0.0 ? l / r : 0.0;
        case BinOp::MOD:
            return r != 0.0 ? std::fmod(l, r) : 0.0;
        case BinOp::LT:
            return l < r ? 1.0 : 0.0;
        case BinOp::GT:
            return l > r ? 1.0 : 0.0;
        case BinOp::LTE:
            return l <= r ? 1.0 : 0.0;
        case BinOp::GTE:
            return l >= r ? 1.0 : 0.0;
        case BinOp::EQ:
            return l == r ? 1.0 : 0.0;
        case BinOp::NEQ:
            return l != r ? 1.0 : 0.0;
        case BinOp::AND:
            return (l != 0.0 && r != 0.0) ? 1.0 : 0.0;
        case BinOp::OR:
            return (l != 0.0 || r != 0.0) ? 1.0 : 0.0;
        }
        return 0.0;
    }

    // ======================== Expression evaluation ========================

    Interpreter::Value Interpreter::eval(const Expr &expr)
    {
        switch (expr.kind)
        {

        case ExprKind::NUMBER:
        {
            auto &num = static_cast<const NumberLiteral &>(expr);
            return Value(num.value);
        }

        case ExprKind::FIELD:
        {
            auto &field = static_cast<const FieldAccess &>(expr);
            std::string name = field.fullName();

            // Check mutable fields first (returns a copy of the array)
            auto fit = m_fields.find(name);
            if (fit != m_fields.end())
            {
                return Value(std::vector<double>(fit->second, fit->second + m_count));
            }

            // Check constants (returns a scalar — no allocation)
            auto cit = m_constants.find(name);
            if (cit != m_constants.end())
            {
                return Value(cit->second);
            }

            throw std::runtime_error("Ink: unknown field or constant '" + name + "'");
        }

        case ExprKind::BINARY:
        {
            auto &bin = static_cast<const BinaryExpr &>(expr);
            Value left = eval(*bin.left);
            Value right = eval(*bin.right);

            // scalar OP scalar → scalar (no allocation)
            if (left.isScalar && right.isScalar)
            {
                return Value(applyBinOp(bin.op, left.scalar, right.scalar));
            }

            // scalar OP vector → reuse right's storage
            if (left.isScalar)
            {
                double s = left.scalar;
                for (size_t i = 0; i < m_count; i++)
                {
                    right.vec[i] = applyBinOp(bin.op, s, right.vec[i]);
                }
                return right;
            }

            // vector OP scalar → reuse left's storage
            if (right.isScalar)
            {
                double s = right.scalar;
                for (size_t i = 0; i < m_count; i++)
                {
                    left.vec[i] = applyBinOp(bin.op, left.vec[i], s);
                }
                return left;
            }

            // vector OP vector → reuse left's storage
            for (size_t i = 0; i < m_count; i++)
            {
                left.vec[i] = applyBinOp(bin.op, left.vec[i], right.vec[i]);
            }
            return left;
        }

        case ExprKind::UNARY:
        {
            auto &un = static_cast<const UnaryExpr &>(expr);
            Value operand = eval(*un.operand);

            if (un.op == UnaryOp::NEG)
            {
                if (operand.isScalar)
                    return Value(-operand.scalar);
                for (size_t i = 0; i < m_count; i++)
                    operand.vec[i] = -operand.vec[i];
                return operand;
            }

            // NOT
            if (operand.isScalar)
                return Value(operand.scalar == 0.0 ? 1.0 : 0.0);
            for (size_t i = 0; i < m_count; i++)
                operand.vec[i] = operand.vec[i] == 0.0 ? 1.0 : 0.0;
            return operand;
        }

        } // switch

        throw std::runtime_error("Ink: unknown expression kind");
    }

    // ======================== Statement execution ========================

    void Interpreter::exec(const Stmt &stmt)
    {
        switch (stmt.kind)
        {
        case StmtKind::IF:
            execIf(static_cast<const IfStmt &>(stmt));
            break;
        case StmtKind::ASSIGN:
            execAssign(static_cast<const AssignStmt &>(stmt));
            break;
        case StmtKind::COMPOUND_ASSIGN:
            execCompoundAssign(static_cast<const CompoundAssignStmt &>(stmt));
            break;
        }
    }

    void Interpreter::execBlock(const Block &block)
    {
        for (const auto &stmt : block.stmts)
        {
            exec(*stmt);
        }
    }

    void Interpreter::execIf(const IfStmt &stmt)
    {
        // Track which sprites haven't been matched by any branch yet
        std::vector<double> remaining = m_activeMask;

        for (const auto &branch : stmt.branches)
        {
            Value cond = eval(*branch.condition);

            // Branch mask = remaining AND condition
            std::vector<double> branchMask(m_count);
            if (cond.isScalar)
            {
                double cv = cond.scalar != 0.0 ? 1.0 : 0.0;
                for (size_t i = 0; i < m_count; i++)
                    branchMask[i] = remaining[i] * cv;
            }
            else
            {
                for (size_t i = 0; i < m_count; i++)
                    branchMask[i] = remaining[i] * (cond.vec[i] != 0.0 ? 1.0 : 0.0);
            }

            // Remove matched sprites from remaining
            for (size_t i = 0; i < m_count; i++)
            {
                if (branchMask[i] > 0.0)
                    remaining[i] = 0.0;
            }

            // Execute branch body under new mask
            m_maskStack.push_back(std::move(m_activeMask));
            m_activeMask = std::move(branchMask);
            execBlock(*branch.body);
            m_activeMask = std::move(m_maskStack.back());
            m_maskStack.pop_back();
        }

        // Optional else branch — uses the remaining mask
        if (stmt.elseBranch)
        {
            m_maskStack.push_back(std::move(m_activeMask));
            m_activeMask = std::move(remaining);
            execBlock(*stmt.elseBranch);
            m_activeMask = std::move(m_maskStack.back());
            m_maskStack.pop_back();
        }
    }

    void Interpreter::execAssign(const AssignStmt &stmt)
    {
        auto it = m_fields.find(stmt.target);
        if (it == m_fields.end())
        {
            throw std::runtime_error(
                "Ink: cannot assign to unknown field '" + stmt.target + "'");
        }

        Value rhs = eval(*stmt.value);
        double *field = it->second;

        if (rhs.isScalar)
        {
            double s = rhs.scalar;
            for (size_t i = 0; i < m_count; i++)
            {
                if (m_activeMask[i] > 0.0)
                    field[i] = s;
            }
        }
        else
        {
            for (size_t i = 0; i < m_count; i++)
            {
                if (m_activeMask[i] > 0.0)
                    field[i] = rhs.vec[i];
            }
        }
    }

    void Interpreter::execCompoundAssign(const CompoundAssignStmt &stmt)
    {
        auto it = m_fields.find(stmt.target);
        if (it == m_fields.end())
        {
            throw std::runtime_error(
                "Ink: cannot assign to unknown field '" + stmt.target + "'");
        }

        Value rhs = eval(*stmt.value);
        double *field = it->second;

        for (size_t i = 0; i < m_count; i++)
        {
            if (m_activeMask[i] <= 0.0)
                continue;

            double rv = rhs.isScalar ? rhs.scalar : rhs.vec[i];

            switch (stmt.op)
            {
            case CompoundOp::ADD_EQ:
                field[i] += rv;
                break;
            case CompoundOp::SUB_EQ:
                field[i] -= rv;
                break;
            case CompoundOp::MUL_EQ:
                field[i] *= rv;
                break;
            case CompoundOp::DIV_EQ:
                if (rv != 0.0)
                    field[i] /= rv;
                break;
            }
        }
    }

} // namespace ink

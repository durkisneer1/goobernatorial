#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>

#include "AST.hpp"

namespace ink
{

    /// Vectorized tree-walking interpreter for Ink scripts.
    ///
    /// Each expression evaluates to a Value — either a scalar (broadcast to all
    /// sprites) or a vector (one element per sprite). Binary operations between
    /// a scalar and a vector reuse the vector's storage in-place, avoiding
    /// unnecessary allocations.
    ///
    /// Conditional blocks (if/elif/else) use boolean masks so that assignments
    /// inside branches only affect the sprites whose condition was true.
    class Interpreter
    {
    public:
        /// Register a mutable SoA field (e.g. "pos.x" → pointer to pos_x data).
        void bindField(const std::string &name, double *data);

        /// Register a read-only constant broadcast to all sprites (e.g. "dt", "PI").
        void setConstant(const std::string &name, double value);

        /// Set the total number of sprites (array length).
        void setCount(size_t count);

        /// Execute a parsed behavior on the currently bound arrays.
        void execute(const BehaviorDecl &behavior);

    private:
        // Internal value type: scalar or per-sprite vector
        struct Value
        {
            std::vector<double> vec;
            double scalar{0.0};
            bool isScalar{false};

            Value() : isScalar(true) {}
            explicit Value(double s) : scalar(s), isScalar(true) {}
            explicit Value(std::vector<double> &&v) : vec(std::move(v)), isScalar(false) {}
        };

        // Expression evaluation
        Value eval(const Expr &expr);

        // Statement execution
        void exec(const Stmt &stmt);
        void execBlock(const Block &block);
        void execIf(const IfStmt &stmt);
        void execAssign(const AssignStmt &stmt);
        void execCompoundAssign(const CompoundAssignStmt &stmt);

        // Active mask (1.0 = sprite participates, 0.0 = masked out)
        std::vector<double> m_activeMask;
        std::vector<std::vector<double>> m_maskStack;

        // Bindings
        std::unordered_map<std::string, double *> m_fields;
        std::unordered_map<std::string, double> m_constants;
        size_t m_count{0};
    };

} // namespace ink

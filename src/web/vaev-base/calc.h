#pragma once
#include <karm-io/emit.h>

#include "numbers.h"

namespace Vaev {

template <typename T>
struct CalcValue {
    enum struct OpCode {
        NOP,
        ADD,
        SUBSTRACT,
        MULTIPLY,
        DIVIDE,
        SIN,
        TAN,
        COS,

        _LEN0
    };

    enum struct OpType {
        FIXED,  // a single value
        SINGLE, // 1 value + 1 OP
        CALC,   // 2 values + 1 OP

        _LEN1
    };

    using Leaf = Box<CalcValue<T>>;

    using Value = Union<None, T, Leaf, Number>;
    OpType type;
    Value lhs = NONE;
    Value rhs = NONE;
    OpCode op = OpCode::NOP;

    constexpr CalcValue()
        : CalcValue(T{}) {
    }

    constexpr CalcValue(Meta::Convertible<T> auto value)
        : type(OpType::FIXED), lhs(T{value}) {
    }

    constexpr CalcValue(Value value)
        : type(OpType::FIXED), lhs(value) {
    }

    constexpr CalcValue(Value lhs, OpCode op)
        : type(OpType::SINGLE), lhs(lhs), op(op) {
    }

    constexpr CalcValue(Value lhs, OpCode op, Value rhs)
        : type(OpType::CALC), lhs(lhs), rhs(rhs), op(op) {
    }

    constexpr bool operator==(OpType type2) const {
        return type == type2;
    }

    void repr(Io::Emit& e) const {
        if (type == OpType::FIXED) {
            e("{}", lhs);
        } else if (type == OpType::SINGLE) {
            e("{} {}", op, lhs);
        } else {
            e("{} {} {}", lhs, op, rhs);
        }
    }
};

} // namespace Vaev

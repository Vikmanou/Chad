#include "chad/net/expr.h"

#include "chad/core/error.h"

namespace chad {

namespace {

std::int64_t wrapAdd(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}

std::int64_t wrapSub(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b));
}

std::int64_t wrapMul(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b));
}

std::int64_t divide(const Code& code, std::int64_t a, std::int64_t b) {
    const bool isDiv = code.op == Operator::Div;

    if (b == 0) throw errorAt(code.line, "divide by zero", isDiv ? "`/` by 0" : "`%` by 0");
    if (b == -1) return isDiv ? wrapSub(0, a) : 0;

    return isDiv ? a / b : a % b;
}

std::int64_t truth(bool value) {
    return value ? 1 : 0;
}

}

std::int64_t evaluate(const Code& code, const std::vector<std::int64_t>& values) {
    switch (code.kind) {
        case Code::Kind::Constant:
            return code.constant;
        case Code::Kind::Slot:
            return values[code.slot];
        case Code::Kind::Unary: {
            const std::int64_t operand = evaluate(code.operands[0], values);
            if (code.op == Operator::Negate) return wrapSub(0, operand);
            return truth(operand == 0);
        }
        case Code::Kind::Binary:
            break;
    }

    const std::int64_t a = evaluate(code.operands[0], values);
    if (code.op == Operator::And) return truth(a != 0 && evaluate(code.operands[1], values) != 0);
    if (code.op == Operator::Or) return truth(a != 0 || evaluate(code.operands[1], values) != 0);

    const std::int64_t b = evaluate(code.operands[1], values);
    switch (code.op) {
        case Operator::Add:
            return wrapAdd(a, b);
        case Operator::Sub:
            return wrapSub(a, b);
        case Operator::Mul:
            return wrapMul(a, b);
        case Operator::Div:
        case Operator::Mod:
            return divide(code, a, b);
        case Operator::Eq:
            return truth(a == b);
        case Operator::Ne:
            return truth(a != b);
        case Operator::Lt:
            return truth(a < b);
        case Operator::Gt:
            return truth(a > b);
        case Operator::Le:
            return truth(a <= b);
        case Operator::Ge:
            return truth(a >= b);
        default:
            return 0;
    }
}

}
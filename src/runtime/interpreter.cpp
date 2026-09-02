#include "chad/runtime/interpreter.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/runtime/io.h"
#include "chad/runtime/utf8.h"
#include "chad/runtime/input.h"

namespace chad {

namespace {

using Stack = std::vector<std::int64_t>;

std::int64_t wrapAdd(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}

std::int64_t wrapSub(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b));
}

std::int64_t wrapMul(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b));
}

std::int64_t applyBinary(const Instruction& instruction, std::int64_t a, std::int64_t b) {
    switch (instruction.op) {
        case Op::Add:
            return wrapAdd(a, b);
        case Op::Sub:
            return wrapSub(a, b);
        case Op::Mul:
            return wrapMul(a, b);
        case Op::Mogs:
            return (a > b) ? 1 : 0;
        default:
            break;
    }

    if (b == 0) {
        throw errorAt(instruction.line, "divide by zero", "`" + opInfo(instruction.op).name + "` by 0");
    } else if (b == -1) {
        return (instruction.op == Op::Div) ? wrapSub(0, a) : 0;
    }
    return (instruction.op == Op::Div) ? (a / b) : (a % b);
}

std::int64_t pop(Stack& stack) {
    const std::int64_t top = stack.back();
    stack.pop_back();
    return top;
}

void runInstruction(const Instruction& instruction, Stack& stack, int& ip) {
    if (opInfo(instruction.op).isMathOp) {
        const std::int64_t b = pop(stack);
        const std::int64_t a = pop(stack);
        stack.push_back(applyBinary(instruction, a, b));
        return;
    }

    switch (instruction.op) {
        case Op::Push:
            stack.push_back(instruction.arg);
            break;
        case Op::Rep:
            stack.push_back(stack.back());
            break;
        case Op::Ghost:
            stack.pop_back();
            break;
        case Op::Pivot:
            std::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
            break;
        case Op::Cope:
            stack.push_back(pop(stack) == 0 ? 1 : 0);
            break;
        case Op::LoopStart:
            if (stack.back() == 0) {
                ip = static_cast<int>(instruction.arg);
            }
            break;
        case Op::LoopEnd:
            if (stack.back() != 0) {
                ip = static_cast<int>(instruction.arg);
            }
            break;
        case Op::Sayc: {
            const std::int64_t value = pop(stack);
            if (!utf8::isValidChar(value)) {
                throw errorAt(instruction.line, "not a char", "`sayc` got " + std::to_string(value));
            }
            writeBytes(utf8::encode(value));
            break;
        }
        case Op::Sayn:
            writeBytes(std::to_string(pop(stack)));
            break;
        case Op::Readc:
            stack.push_back(readCodePoint());
            break;
        case Op::Readn:
            stack.push_back(readNumber());
            break;
        default:
            break;
    }
}

}

void run(const Program& program) {
    Stack stack;
    int instructionPointer = 0;

    while (instructionPointer < program.instructions.size()) {
        const Instruction& instruction = program.instructions[instructionPointer];

        const OpInfo& info = opInfo(instruction.op);
        if (stack.size() < info.numbersNeeded) {
            const std::string detail = "`" + info.name + "` wanted " + std::to_string(info.numbersNeeded) + ", found " + std::to_string(stack.size());
            throw errorAt(instruction.line, "stack empty", detail);
        }

        ++instructionPointer;

        runInstruction(instruction, stack, instructionPointer);
    }

    flushOutput();
}

}

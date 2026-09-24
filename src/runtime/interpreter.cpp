#include "chad/runtime/interpreter.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/runtime/io.h"

namespace chad {

namespace {

using Stack = std::vector<std::int64_t>;

std::int64_t pop(Stack& stack) {
    const std::int64_t top = stack.back();
    stack.pop_back();
    return top;
}

}

void run(const Program& program) {
    Stack stack;
    int instructionPointer = 0;

    while (instructionPointer < program.instructions.size()) {
        const Instruction& instruction = program.instructions[instructionPointer];
        ++instructionPointer;

    }
}

}

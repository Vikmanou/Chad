#include "chad/compiler/program.h"

#include <optional>

#include "chad/core/error.h"
#include "chad/core/text.h"
#include "chad/compiler/instruction.h"
#include "chad/compiler/lexer.h"

namespace chad {

namespace {

Instruction fromToken(const Token& token) {
    Instruction instruction;
    instruction.line = token.line;

    if (token.kind == TokenKind::Number) {
        instruction.op = Op::Push;
        instruction.arg = token.value;
        return instruction;
    }

    const std::optional<Op> op = findOp(toLowercase(token.text));
    if (!op) {
        throw errorAt(token.line, "unknown word `" + token.text + "`");
    }

    instruction.op = *op;
    return instruction;
}

void linkBrackets(std::vector<Instruction>& instructions) {
    std::vector<int> open;

    for (int i = 0; i < instructions.size(); ++i) {
        if (instructions[i].op == Op::LoopStart) {
            open.push_back(i);
        } else if (instructions[i].op == Op::LoopEnd) {
            if (open.empty()) {
                throw errorAt(instructions[i].line, "unmatched bracket", "`]` here never opens");
            }

            const int start = open.back();
            open.pop_back();
            instructions[start].arg = i + 1;
            instructions[i].arg = start + 1;
        }
    }

    if (!open.empty()) {
        throw errorAt(instructions[open.back()].line, "unmatched bracket", "`[` here never closes");
    }
}

}

Program compile(const std::string& source) {
    Program program;

    for (const Token& token : lex(source)) {
        program.instructions.push_back(fromToken(token));
    }

    linkBrackets(program.instructions);

    return program;
}

}
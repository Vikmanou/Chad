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

}

Program compile(const std::string& source) {
    Program program;

    for (const Token& token : lex(source)) {
        program.instructions.push_back(fromToken(token));
    }

    return program;
}

}
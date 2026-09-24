#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace chad {

enum class Op {
    Push,
    Rep,
    Ghost,
    Pivot,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Cope,
    Mogs,
    LoopStart,
    LoopEnd,
    Sayc,
    Sayn,
    Readc,
    Readn,
};

struct OpInfo {
    std::string name;
    int numbersNeeded;
    bool isMathOp;
};

struct Instruction {
    Op op = Op::Push;
    std::int64_t arg = 0;
    int line = 1;
};

const OpInfo& opInfo(Op op);

std::optional<Op> findOp(const std::string& word);

}

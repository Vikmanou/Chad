#include "chad/lang/instruction.h"

#include <unordered_map>

namespace chad {

namespace {

const std::unordered_map<Op, OpInfo> OPS = {
    {Op::Push, {"push", 0, false}},   {Op::Rep, {"rep", 1, false}},
    {Op::Ghost, {"ghost", 1, false}}, {Op::Pivot, {"pivot", 2, false}},
    {Op::Add, {"+", 2, true}},        {Op::Sub, {"-", 2, true}},
    {Op::Mul, {"*", 2, true}},        {Op::Div, {"/", 2, true}},
    {Op::Mod, {"%", 2, true}},        {Op::Cope, {"cope", 1, false}},
    {Op::Mogs, {"mogs", 2, true}},    {Op::LoopStart, {"[", 1, false}},
    {Op::LoopEnd, {"]", 1, false}},   {Op::Sayc, {"sayc", 1, false}},
    {Op::Sayn, {"sayn", 1, false}},   {Op::Readc, {"readc", 0, false}},
    {Op::Readn, {"readn", 0, false}},
};

}

const OpInfo& opInfo(Op op) {
    return OPS.at(op);
}

std::optional<Op> findOp(const std::string& word) {
    for (const auto& [op, info] : OPS) {
        if (op != Op::Push && info.name == word) return op;
    }
    return std::nullopt;
}

}

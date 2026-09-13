#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chad/lang/ast.h"

namespace chad {

using Breed = int;

// the breeds every program has
namespace breed {
constexpr Breed Number = 0;
constexpr Breed World = 1;
constexpr Breed Chad = 2;
constexpr Breed Rep = 3;
constexpr Breed Ghost = 4;
constexpr Breed Say = 5;
constexpr Breed Read = 6;
constexpr Breed Eof = 7;
constexpr Breed Cons = 8;
constexpr Breed Nil = 9;
}

struct BreedInfo {
    std::string name;
    int valueCount = 0;
    int armCount = 0;
    int line = 0; // where it was first seen. 0 for built-ins
};

// a value to compute when a rule fires
struct Code {
    enum class Kind {
        Constant,
        Slot,
        Unary,
        Binary
    };
    Kind kind = Kind::Constant;
    std::int64_t constant = 0;
    int slot = 0; // which of the facing Chads' values
    Operator op = Operator::Add;
    std::vector<Code> operands;
    int line = 0;
};

// one end of a wire in a template
struct End {
    bool outside = false;
    int index = 0; // which outside port or which new Chad
    int slot = 0; // port of the new Chad: 0 is the face, 1 the arms
};

struct NewChad {
    Breed breed = 0;
    std::vector<Code> values;
    int line = 0;
};

// face off result
struct Template {
    std::vector<NewChad> chads;
    std::vector<std::pair<End, End>> links;
};

struct RuleCase {
    bool hasCondition = false;
    Code condition;
    Template result;
    int line = 0;
};

struct Rule {
    Breed left = 0;
    Breed right = 0;
    std::vector<RuleCase> cases;
    int line = 0;
};

struct Program {
    std::vector<BreedInfo> breeds;
    std::vector<Rule> rules;
    std::vector<int> ruleTable;
    Template main;
    bool usesWorld = false;
    int mainLine = 0;

    int findRule(Breed a, Breed b) const {
        return ruleTable[static_cast<std::size_t>(a) * breeds.size() + static_cast<std::size_t>(b)];
    }
};

}
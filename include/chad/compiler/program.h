#pragma once

#include <string>
#include <vector>

#include "chad/compiler/instruction.h"

namespace chad {

struct Program {
    std::vector<Instruction> instructions;
};

Program compile(const std::string& source);

}

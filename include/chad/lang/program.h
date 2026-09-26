#pragma once

#include <string>
#include <vector>

#include "chad/lang/instruction.h"

namespace chad {

struct Program {
    std::vector<Instruction> instructions;
};

Program compile(const std::string& source);

}

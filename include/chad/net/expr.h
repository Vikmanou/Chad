#pragma once

#include <cstdint>
#include <vector>

#include "chad/lang/program.h"

namespace chad {

std::int64_t evaluate(const Code& code, const std::vector<std::int64_t>& values);

}
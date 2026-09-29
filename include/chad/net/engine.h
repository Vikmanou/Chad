#pragma once

#include <cstdint>

#include "chad/lang/program.h"

namespace chad {

class Io;

std::uint64_t run(const Program& program, Io& io);

}
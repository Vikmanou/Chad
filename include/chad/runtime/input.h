#pragma once

#include <cstdint>
#include <optional>

namespace chad {

class Io;

std::optional<std::int64_t> readCodePoint(Io& io);

}
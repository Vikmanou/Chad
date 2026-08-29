#pragma once

#include <optional>
#include <string>

namespace chad {

std::optional<std::string> readFile(const std::string& path);

}

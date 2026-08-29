#include "chad/core/file.h"

#include <fstream>
#include <sstream>

namespace chad {

std::optional<std::string> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return std::nullopt;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}
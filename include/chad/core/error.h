#pragma once

#include <stdexcept>
#include <string>

namespace chad {

struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

Error errorAt(int line, const std::string& kind, const std::string& detail = "");

}

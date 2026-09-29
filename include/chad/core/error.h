#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

#include "chad/lang/token.h"

namespace chad {

struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;

    int line = 0;
};

Error errorAt(int line, const std::string& kind, const std::string& detail = "");

std::string lineName(int line);

std::string preludeLineName(int line);

std::string plural(std::size_t count, const char* word);

std::string shape(std::size_t values, std::size_t arms);

std::string describe(const Token& token);

}
#pragma once

#include <string>

#include "chad/compiler/token.h"

namespace chad {

TokenList lex(const std::string& source);

}

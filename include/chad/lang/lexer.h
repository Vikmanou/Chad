#pragma once

#include <string>

#include "chad/lang/token.h"

namespace chad {

TokenList lex(const std::string& source);

}

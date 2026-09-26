#pragma once

#include "chad/lang/ast.h"
#include "chad/lang/token.h"

namespace chad {

ast::Source parse(const TokenList& tokens);

}
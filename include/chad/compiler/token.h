#pragma once

#include <string>
#include <vector>

namespace chad {

enum class TokenKind {
    Number,
    Word,
};

struct Token {
    TokenKind kind = TokenKind::Word;
    std::string text;
    std::int64_t value = 0;
    int line = 1;
};

using TokenList = std::vector<Token>;

}

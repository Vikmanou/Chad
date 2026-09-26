#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chad {

enum class TokenKind {
    Breed, // starts with a capital
    Name, // starts lowercase
    Number,
    Char,
    String,
    Symbol, // ( ) [ ] , ~ => and operators
    Newline,
    End,
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    std::int64_t value = 0; // Number and Char
    std::vector<std::int64_t> chars; // String, as code points
    int line = 1;
};

using TokenList = std::vector<Token>;

}
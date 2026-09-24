#include "chad/compiler/lexer.h"

#include <string>

#include "chad/core/text.h"
#include "chad/compiler/token.h"

namespace chad {

// nested helper namespace for lexer
namespace {

const std::string COMMENT_WORD = "ngl";

bool isNewLine(const std::string& source, int i) {
    if (source[i] == '\n') {
        return true;
    } else if (source[i] == '\r') {
        return i + 1 >= source.size() || source[i + 1] != '\n';
    }
    return false;
}

}

TokenList lex(const std::string& source) {
    TokenList tokens;

    int idx = 0;

    int line = 1;

    while (idx < source.size()) {
        if (isSpace(source[idx])) {
            if (isNewLine(source, idx)) {
                line++;
            }
            idx++;
            continue;
        }

        const int tokenStart = idx;
        while (idx < source.size() && !isSpace(source[idx])) {
            ++idx;
        }

        const std::string lexeme = source.substr(tokenStart, idx - tokenStart);
        if (toLowercase(lexeme) == COMMENT_WORD) {
            while (i < source.size() && !isNewLine(source, i)) {
                ++i;
            }
            continue;
        }

        // tokens.push_back(  );
    }

    return tokens;
}

}
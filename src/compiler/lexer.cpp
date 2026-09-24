#include "chad/compiler/lexer.h"

#include <cstdint>
#include <optional>
#include <string>

#include "chad/core/error.h"
#include "chad/core/text.h"
#include "chad/compiler/token.h"

namespace chad {

// nested helper namespace for lexer
namespace {

const std::string COMMENT_WORD = "ngl";
const std::string UTF8_BOM = "\xEF\xBB\xBF"; // some editors (notepad) put these bytes to mark UTF-8

bool startsAsNumber(const std::string& lexeme) {
    if (isDigit(lexeme[0]))
        return true;
    return lexeme[0] == '-' && lexeme.size() > 1 && isDigit(lexeme[1]);
}

std::optional<std::int64_t> parseInteger(const std::string& lexeme) {
    int i = 0;

    bool negative = false;
    if (lexeme[0] == '-') {
        if (lexeme.size() == 1) return std::nullopt;

        negative = true;
        i = 1;
    }

    // unsigned so a literal too big for 64 bits wraps instead of overflowing
    std::uint64_t value = 0;
    for (; i < lexeme.size(); ++i) {
        if (!isDigit(lexeme[i])) return std::nullopt;
        value = value * 10 + static_cast<std::uint64_t>(lexeme[i] - '0');
    }

    return static_cast<std::int64_t>(negative ? -value : value);
}

bool isNewLine(const std::string& source, int i) {
    if (source[i] == '\n') {
        return true;
    } else if (source[i] == '\r') {
        return i + 1 >= source.size() || source[i + 1] != '\n';
    }
    return false;
}

Token classify(const std::string& lexeme, int line) {
    Token token;
    token.text = lexeme;
    token.line = line;

    if (startsAsNumber(lexeme)) {
        const std::optional<std::int64_t> value = parseInteger(lexeme);
        if (!value) {
            throw errorAt(line, "unknown word `" + lexeme + "`", "not a number; tokens need spaces around them");
        }
        token.kind = TokenKind::Number;
        token.value = *value;
    } else {
        token.kind = TokenKind::Word;
    }

    return token;
}

}

TokenList lex(const std::string& source) {
    TokenList tokens;

    int idx = (source.compare(0, UTF8_BOM.size(), UTF8_BOM) == 0) ? UTF8_BOM.size() : 0;

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
            while (idx < source.size() && !isNewLine(source, idx)) {
                ++idx;
            }
            continue;
        }

        tokens.push_back(classify(lexeme, line));
    }

    return tokens;
}

}
#include "chad/core/error.h"

#include "chad/core/text.h"
#include "prelude.h"

namespace chad {

Error errorAt(int line, const std::string& kind, const std::string& detail) {
    std::string message = lineName(line) + ": " + kind;
    if (!detail.empty()) {
        message += " -- " + detail;
    }
    Error error(message);
    error.line = line;
    return error;
}

std::string lineName(int line) {
    if (line < 0) return preludeLineName(-line);

    return "line " + std::to_string(line);
}

std::string preludeLineName(int line) {
    for (const PreludeFile& file : PRELUDE_FILES) {
        const int lines = countLines(file.source);
        if (line <= lines) return std::string(file.name) + " line " + std::to_string(line);

        line -= lines;
    }

    return "prelude line " + std::to_string(line);
}

std::string plural(std::size_t count, const char* word) {
    if (count == 0) {
        return std::string("no ") + word + "s";
    } else if (count == 1) {
        return std::string("1 ") + word;
    }
    return std::to_string(count) + " " + word + "s";
}

std::string shape(std::size_t values, std::size_t arms) {
    return plural(values, "value") + " and " + plural(arms, "arm");
}

std::string describe(const Token& token) {
    switch (token.kind) {
        case TokenKind::End:
            return "the end of the file";
        case TokenKind::Newline:
            return "the end of the line";
        case TokenKind::String:
            return "a string";
        default:
            return "`" + token.text + "`";
    }
}

}
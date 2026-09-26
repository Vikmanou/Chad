#include "chad/core/text.h"

namespace chad {

bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

std::string toLowercase(std::string str) {
    for (char& c : str) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return str;
}

int countLines(const std::string& text) {
    int lines = 1;
    for (const char c : text) {
        if (c == '\n') lines++;
    }

    return lines;
}

}
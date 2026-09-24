#pragma once

#include <string>

namespace chad {

bool isSpace(char c);

bool isDigit(char c);

std::string toLowercase(std::string word);

int countLines(const std::string& text);

}
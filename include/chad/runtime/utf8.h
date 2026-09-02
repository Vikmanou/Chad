#pragma once

#include <cstdint>
#include <string>

namespace chad::utf8 {

bool isValidChar(std::int64_t codePoint);

int charByteCount(unsigned char firstByte);

bool isContinuationByte(unsigned char byte);

std::int64_t firstByteBits(unsigned char firstByte, int byteCount);

std::string encode(std::int64_t codePoint);

}

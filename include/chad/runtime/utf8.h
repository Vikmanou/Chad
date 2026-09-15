#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace chad::utf8 {

bool isValidChar(std::int64_t codePoint);

std::size_t charByteCount(unsigned char firstByte);

bool isContinuationByte(unsigned char byte);

std::int64_t firstByteBits(unsigned char firstByte, std::size_t byteCount);

std::string encode(std::int64_t codePoint);

}
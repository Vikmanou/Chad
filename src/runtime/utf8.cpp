#include "chad/runtime/utf8.h"

namespace chad::utf8 {

bool isValidChar(std::int64_t codePoint) {
    if (codePoint < 0 || codePoint > 0x10FFFF) {
        return false;
    }
    return codePoint < 0xD800 || codePoint > 0xDFFF;
}

int charByteCount(unsigned char firstByte) {
    if (firstByte < 0x80) {
        return 1;
    } else if ((firstByte & 0xE0) == 0xC0) {
        return 2;
    } else if ((firstByte & 0xF0) == 0xE0) {
        return 3;
    } else if ((firstByte & 0xF8) == 0xF0) {
        return 4;
    }
    return 0;
}

bool isContinuationByte(unsigned char byte) {
    return (byte & 0xC0) == 0x80;
}

std::int64_t firstByteBits(unsigned char firstByte, int byteCount) {
    return firstByte & (0x7F >> byteCount);
}

std::string encode(std::int64_t codePoint) {
    const std::uint32_t value = static_cast<std::uint32_t>(codePoint);
    std::string bytes;

    if (value < 0x80) {
        bytes.push_back(static_cast<char>(value));
    } else if (value < 0x800) {
        bytes.push_back(static_cast<char>(0xC0 | (value >> 6)));
        bytes.push_back(static_cast<char>(0x80 | (value & 0x3F)));
    } else if (value < 0x10000) {
        bytes.push_back(static_cast<char>(0xE0 | (value >> 12)));
        bytes.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
        bytes.push_back(static_cast<char>(0x80 | (value & 0x3F)));
    } else {
        bytes.push_back(static_cast<char>(0xF0 | (value >> 18)));
        bytes.push_back(static_cast<char>(0x80 | ((value >> 12) & 0x3F)));
        bytes.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
        bytes.push_back(static_cast<char>(0x80 | (value & 0x3F)));
    }

    return bytes;
}

}
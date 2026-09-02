#include "chad/runtime/input.h"

#include "chad/core/text.h"
#include "chad/runtime/io.h"
#include "chad/runtime/utf8.h"

namespace chad {

std::int64_t readCodePoint() {
    const int firstByte = readByte();
    if (firstByte < 0) return 0;

    const int byteCount = utf8::charByteCount(static_cast<unsigned char>(firstByte));
    if (byteCount <= 1) return firstByte;

    std::int64_t codePoint = utf8::firstByteBits(static_cast<unsigned char>(firstByte), byteCount);
    for (int i = 1; i < byteCount; ++i) {
        const int nextByte = readByte();
        if (nextByte < 0 || !utf8::isContinuationByte(static_cast<unsigned char>(nextByte))) {
            return firstByte;
        }
        codePoint = (codePoint << 6) | (nextByte & 0x3F);
    }

    return codePoint;
}

std::int64_t readNumber() {
    int byte = readByte();
    while (byte >= 0 && isSpace(static_cast<char>(byte))) {
        byte = readByte();
    }
    if (byte < 0) return 0;

    bool negative = false;
    if (byte == '-') {
        negative = true;
        byte = readByte();
    }

    std::uint64_t value = 0;
    while (byte >= 0 && isDigit(static_cast<char>(byte))) {
        value = value * 10 + static_cast<std::uint64_t>(byte - '0');
        byte = readByte();
    }

    return static_cast<std::int64_t>(negative ? 0 - value : value);
}

}

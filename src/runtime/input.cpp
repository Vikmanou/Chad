#include "chad/runtime/input.h"

#include "chad/core/text.h"
#include "chad/runtime/io.h"
#include "chad/runtime/utf8.h"

#include <optional>

namespace chad {

std::optional<std::int64_t> readCodePoint() {
    const int firstByte = readByte();
    if (firstByte < 0) return std::nullopt;

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

}
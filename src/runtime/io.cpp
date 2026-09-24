#include "chad/runtime/io.h"

#include <cstdio>

namespace chad {

int readByte() {
    const int byte = std::fgetc(stdin);
    return (byte == EOF) ? -1 : byte;
}

void writeBytes(const std::string& bytes) {
    std::fwrite(bytes.data(), 1, bytes.size(), stdout);
}

void flushOutput() {
    std::fflush(stdout);
}

}

#include "chad/runtime/io.h"

#include <cstddef>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace chad {

namespace {

const std::size_t bufferSize = 1 << 16;

std::string output;
char input[bufferSize];
std::size_t inputStart = 0;
std::size_t inputEnd = 0;

long readSome(char* bytes, std::size_t count) {
#ifdef _WIN32
    return _read(0, bytes, static_cast<unsigned>(count));
#else
    return static_cast<long>(read(0, bytes, count));
#endif
}

long writeSome(const char* bytes, std::size_t count) {
#ifdef _WIN32
    return _write(1, bytes, static_cast<unsigned>(count));
#else
    return static_cast<long>(write(1, bytes, count));
#endif
}

}

int readByte() {
    if (inputStart == inputEnd) {
        flushOutput();

        const long got = readSome(input, bufferSize);
        if (got <= 0) return -1;

        inputStart = 0;
        inputEnd = static_cast<std::size_t>(got);
    }

    return static_cast<unsigned char>(input[inputStart++]);
}

void writeBytes(const std::string& bytes) {
    output += bytes;
    if (output.size() >= bufferSize) flushOutput();
}

void flushOutput() {
    std::size_t done = 0;
    while (done < output.size()) {
        const long wrote = writeSome(output.data() + done, output.size() - done);
        if (wrote <= 0) break;
        done += static_cast<std::size_t>(wrote);
    }

    output.clear();
}

}
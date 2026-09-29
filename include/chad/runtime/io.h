#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace chad {

struct IoHandlers {
    std::function<long(char*, std::size_t)> read;
    std::function<long(const char*, std::size_t)> write;
    std::function<bool()> ready;
};

IoHandlers standardIo();

class Io {
public:
    explicit Io(IoHandlers handlers = standardIo());

    int readByte();

    void writeBytes(const std::string& bytes);

    void flushOutput();

    bool inputReady() const;

private:
    IoHandlers handlers;
    std::string output;
    std::vector<char> input;
    std::size_t inputStart = 0;
    std::size_t inputEnd = 0;
};

}
#pragma once

#include <cstdint>
#include <memory>

#include "chad/lang/program.h"

namespace chad {

class Io;
class Engine;
struct NetObserver;

enum class Status { Running, Done, NeedInput };

class Machine {
public:
    Machine(const Program& program, Io& io, NetObserver* observer = nullptr);
    ~Machine();

    void start();
    Status step();
    Status runFor(std::uint64_t maxSteps);
    std::uint64_t interactions() const;

private:
    std::unique_ptr<Engine> engine;
};

std::uint64_t run(const Program& program, Io& io);

}
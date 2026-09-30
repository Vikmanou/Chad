#pragma once

#include <cstddef>

#include "chad/net/net.h"

namespace chad {

struct NetObserver {
    virtual ~NetObserver() = default;

    virtual void added(std::size_t id, const Node& node) = 0;
    virtual void removed(std::size_t id) = 0;
    virtual void linked(Port a, Port b) = 0;

    // a and b face off. `line` is the rule case that fired or the Chad behind a built-in.
    virtual void interaction(std::size_t a, std::size_t b, int line) = 0;
};

}
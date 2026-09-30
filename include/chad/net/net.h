#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "chad/lang/program.h"

namespace chad {

struct Port {
    std::size_t node = 0;
    std::size_t slot = 0;
};

struct Node {
    Breed breed = 0;
    int line = 0;
    std::uint64_t label = 0;
    std::vector<std::int64_t> values;
    std::vector<Port> ports;
};

class Net {
public:
    std::size_t add(Breed breed, std::size_t armCount, int line);

    void remove(std::size_t node);

    Node& operator[](std::size_t node) {
        return nodes[node];
    }

    const Node& operator[](std::size_t node) const {
        return nodes[node];
    }

    void link(Port a, Port b);

    bool peekFaceOff(std::pair<std::size_t, std::size_t>& faceOff);

    bool nextFaceOff(std::pair<std::size_t, std::size_t>& faceOff);

private:
    std::vector<Node> nodes;
    std::vector<std::size_t> freeNodes;
    std::vector<std::pair<std::size_t, std::size_t>> faceOffs;

    bool facing(std::size_t a, std::size_t b) const;
};

}
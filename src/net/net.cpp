#include "chad/net/net.h"

namespace chad {

std::size_t Net::add(Breed breed, std::size_t armCount, int line) {
    std::size_t id = nodes.size();
    if (freeNodes.empty()) {
        nodes.emplace_back();
    } else {
        id = freeNodes.back();
        freeNodes.pop_back();
    }

    Node& node = nodes[id];
    node.breed = breed;
    node.line = line;
    node.label = 0;
    node.values.clear();
    node.ports.assign(armCount + 1, Port{});

    return id;
}

void Net::remove(std::size_t node) {
    nodes[node].ports.clear();
    freeNodes.push_back(node);
}

}
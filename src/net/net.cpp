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

void Net::link(Port a, Port b) {
    nodes[a.node].ports[a.slot] = b;
    nodes[b.node].ports[b.slot] = a;

    if (a.slot == 0 && b.slot == 0) faceOffs.emplace_back(a.node, b.node);
}

bool Net::facing(std::size_t a, std::size_t b) const {
    if (nodes[a].ports.empty() || nodes[b].ports.empty()) return false;

    const Port faceA = nodes[a].ports[0];
    const Port faceB = nodes[b].ports[0];
    return faceA.node == b && faceA.slot == 0 && faceB.node == a && faceB.slot == 0;
}

bool Net::peekFaceOff(std::pair<std::size_t, std::size_t>& faceOff) {
    while (!faceOffs.empty()) {
        faceOff = faceOffs.back();
        if (facing(faceOff.first, faceOff.second)) return true;

        faceOffs.pop_back();
    }

    return false;
}

void Net::popFaceOff() {
    faceOffs.pop_back();
}

}
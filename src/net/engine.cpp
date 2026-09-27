#include "chad/net/engine.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/lang/program.h"
#include "chad/net/net.h"
#include "chad/runtime/io.h"
#include "chad/runtime/utf8.h"

namespace chad {

namespace {

struct Inside {
    bool outside = false;
    std::size_t index = 0;
    Port port;
};

class Engine {
public:
    explicit Engine(const Program& program) : program(program) {}

    void run() {
        start();

        std::pair<std::size_t, std::size_t> faceOff;
        while (net.nextFaceOff(faceOff)) {
            react(faceOff.first, faceOff.second);
        }

        flushOutput();
    }

private:
    const Program& program;
    Net net;

    std::vector<Port> outsidePorts;
    std::vector<Inside> insides;
    std::vector<std::size_t> made;

    std::string describe(std::size_t node) const {
        const Node& chad = net[node];
        if (chad.breed == breed::Number) return "`" + std::to_string(chad.values[0]) + "`";

        std::string text = "`" + program.breeds[chad.breed].name;
        if (!chad.values.empty()) {
            text += "[";
            for (std::size_t i = 0; i < chad.values.size(); i++) {
                if (i > 0) text += ", ";
                text += std::to_string(chad.values[i]);
            }
            text += "]";
        }

        return text + "`";
    }

    std::int64_t evaluate(const Code& code) const {
        if (code.kind == Code::Kind::Constant) return code.constant;
        if (code.kind == Code::Kind::Unary && code.op == Operator::Negate) return -evaluate(code.operands[0]);

        throw errorAt(code.line, "can't work out this value yet");
    }

    void start() {
        outsidePorts.clear();
        if (program.usesWorld) {
            const std::size_t world = net.add(breed::World, 0, program.mainLine);
            outsidePorts.push_back(Port{world, 0});
        }
        insides.assign(outsidePorts.size(), Inside{});

        build(program.main);

        for (std::size_t i = 0; i < outsidePorts.size(); i++) {
            net.link(outsidePorts[i], insides[i].port);
        }
    }

    Port port(End end) const {
        return Port{made[end.index], end.slot};
    }

    void build(const Template& result) {
        made.clear();
        for (const NewChad& chad : result.chads) {
            const std::size_t node = net.add(chad.breed, program.breeds[chad.breed].armCount, chad.line);
            made.push_back(node);
            for (const Code& code : chad.values) {
                net[node].values.push_back(evaluate(code));
            }
        }

        for (const auto& [x, y] : result.links) {
            if (x.outside && y.outside) {
                insides[x.index] = Inside{true, y.index, Port{}};
                insides[y.index] = Inside{true, x.index, Port{}};
            } else if (x.outside) {
                insides[x.index] = Inside{false, 0, port(y)};
            } else if (y.outside) {
                insides[y.index] = Inside{false, 0, port(x)};
            } else {
                net.link(port(x), port(y));
            }
        }
    }

    void react(std::size_t a, std::size_t b) {
        const Breed first = net[a].breed;
        const Breed second = net[b].breed;

        if (first == breed::Ghost) return erase(a, b);
        if (second == breed::Ghost) return erase(b, a);
        if (first == breed::World) return meetWorld(b, a);
        if (second == breed::World) return meetWorld(a, b);

        throw errorAt(net[a].line, "no rule for " + describe(a) + " vs " + describe(b));
    }

    void erase(std::size_t ghost, std::size_t victim) {
        const int line = net[ghost].line;
        const std::vector<Port> ports = net[victim].ports;

        net.remove(ghost);
        net.remove(victim);

        for (std::size_t slot = 1; slot < ports.size(); slot++) {
            const std::size_t newGhost = net.add(breed::Ghost, 0, line);
            net.link(Port{newGhost, 0}, ports[slot]);
        }
    }

    void meetWorld(std::size_t chad, std::size_t world) {
        if (net[chad].breed == breed::Say) return say(chad, world);

        throw errorAt(net[chad].line, describe(chad) + " faced the World", "only `Say` and `Read` can");
    }

    void say(std::size_t chad, std::size_t world) {
        const std::int64_t c = net[chad].values[0];
        if (!utf8::isValidChar(c)) throw errorAt(net[chad].line, "not a char", "`Say` got " + std::to_string(c));

        writeBytes(utf8::encode(c));

        const Port next = net[chad].ports[1];
        net.remove(chad);
        net.link(Port{world, 0}, next);
    }
};

}

void run(const Program& program) {
    Engine(program).run();
}

}
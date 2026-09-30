#include "chad/net/engine.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/lang/program.h"
#include "chad/net/expr.h"
#include "chad/net/net.h"
#include "chad/runtime/input.h"
#include "chad/runtime/io.h"
#include "chad/runtime/utf8.h"

namespace chad {

namespace {

struct Inside {
    bool outside = false;
    std::size_t index = 0;
    Port port;
};

}

class Engine {
public:
    Engine(const Program& program, Io& io) : program(program), io(io) {}

    std::uint64_t interactions = 0;

    void start() {
        collectOutside({});
        bound.clear();

        if (program.usesWorld) {
            const std::size_t world = net.add(breed::World, 0, program.mainLine);
            outsidePorts.push_back(Port{world, 0});
        }
        insides.assign(outsidePorts.size(), Inside{});

        build(program.main);
        rewire();
    }

    Status step() {
        std::pair<std::size_t, std::size_t> faceOff;
        if (!net.peekFaceOff(faceOff)) {
            io.flushOutput();
            return Status::Done;
        }

        if (hearing(faceOff.first, faceOff.second) && !io.inputReady()) {
            io.flushOutput();
            return Status::NeedInput;
        }

        net.popFaceOff();
        react(faceOff.first, faceOff.second);
        interactions++;

        return Status::Running;
    }

private:
    const Program& program;
    Io& io;
    Net net;

    std::vector<Port> outsidePorts;
    std::vector<Inside> insides;
    std::vector<std::size_t> made;
    std::vector<std::size_t> dying;
    std::vector<bool> visited;
    std::vector<std::int64_t> bound;
    std::uint64_t nextLabel = 1;

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

    bool hearing(std::size_t a, std::size_t b) const {
        const Breed first = net[a].breed;
        const Breed second = net[b].breed;
        return (first == breed::Hear && second == breed::World) || (first == breed::World && second == breed::Hear);
    }

    void collectOutside(const std::vector<std::size_t>& nodes) {
        dying = nodes;

        outsidePorts.clear();

        for (const std::size_t node : dying) {
            for (std::size_t slot = 1; slot < net[node].ports.size(); slot++) {
                outsidePorts.push_back(net[node].ports[slot]);
            }
        }

        insides.assign(outsidePorts.size(), Inside{});
    }

    bool dyingArm(Port port, std::size_t& arm) const {
        std::size_t offset = 0;
        for (const std::size_t node : dying) {
            if (port.node == node && port.slot > 0) {
                arm = offset + port.slot - 1;
                return true;
            }

            offset += net[node].ports.size() - 1;
        }

        return false;
    }

    void rewire() {
        visited.assign(outsidePorts.size(), false);

        for (std::size_t i = 0; i < outsidePorts.size(); i++) {
            if (visited[i]) continue;
            visited[i] = true;

            const std::optional<Port> from = follow(i, true);
            const std::optional<Port> to = follow(i, false);
            if (from && to) net.link(*from, *to);
        }
    }

    std::optional<Port> follow(std::size_t index, bool outward) {
        while (true) {
            std::size_t next = 0;
            if (outward) {
                const Port port = outsidePorts[index];
                if (!dyingArm(port, next)) return port;
            } else {
                const Inside& inside = insides[index];
                if (!inside.outside) return inside.port;
                next = inside.index;
            }

            if (visited[next]) return std::nullopt;
            visited[next] = true;
            index = next;
            outward = !outward;
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
                net[node].values.push_back(evaluate(code, bound));
            }

            if (chad.breed == breed::Rep) net[node].label = nextLabel++;
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
        if (first == breed::Rep && second == breed::Rep && net[a].label == net[b].label) return annihilate(a, b);
        if (first == breed::Rep) return commute(a, b);
        if (second == breed::Rep) return commute(b, a);

        const int index = program.findRule(first, second);
        if (index < 0) {
            const std::size_t named = first >= second ? a : b;
            const std::size_t other = first >= second ? b : a;
            throw errorAt(net[named].line, "no rule for " + describe(named) + " vs " + describe(other));
        }

        apply(program.rules[static_cast<std::size_t>(index)], a, b);
    }

    void apply(const Rule& rule, std::size_t a, std::size_t b) {
        if (net[a].breed != rule.left) std::swap(a, b);

        bound = net[a].values;
        bound.insert(bound.end(), net[b].values.begin(), net[b].values.end());

        collectOutside({a, b});

        const RuleCase* chosen = nullptr;
        for (const RuleCase& ruleCase : rule.cases) {
            if (!ruleCase.hasCondition || evaluate(ruleCase.condition, bound) != 0) {
                chosen = &ruleCase;
                break;
            }
        }
        if (chosen == nullptr) throw errorAt(rule.line, "no case fits " + describe(a) + " vs " + describe(b));

        build(chosen->result);
        rewire();
        net.remove(a);
        net.remove(b);
    }

    void annihilate(std::size_t a, std::size_t b) {
        collectOutside({a, b});

        const std::size_t arms = net[a].ports.size() - 1;
        for (std::size_t i = 0; i < arms; i++) {
            insides[i] = Inside{true, arms + i, Port{}};
            insides[arms + i] = Inside{true, i, Port{}};
        }

        rewire();
        net.remove(a);
        net.remove(b);
    }

    void commute(std::size_t rep, std::size_t other) {
        collectOutside({rep, other});

        const Breed kind = net[other].breed;
        const std::size_t arms = net[other].ports.size() - 1;
        const std::vector<std::int64_t> values = net[other].values;
        const std::uint64_t otherLabel = net[other].label;
        const std::uint64_t repLabel = net[rep].label;
        const int otherLine = net[other].line;
        const int repLine = net[rep].line;

        std::size_t copies[2];
        for (std::size_t k = 0; k < 2; k++) {
            copies[k] = net.add(kind, arms, otherLine);
            net[copies[k]].values = values;
            net[copies[k]].label = otherLabel;
            insides[k].port = Port{copies[k], 0};
        }

        for (std::size_t i = 0; i < arms; i++) {
            const std::size_t split = net.add(breed::Rep, 2, repLine);
            net[split].label = repLabel;
            insides[2 + i].port = Port{split, 0};

            net.link(Port{split, 1}, Port{copies[0], i + 1});
            net.link(Port{split, 2}, Port{copies[1], i + 1});
        }

        rewire();
        net.remove(rep);
        net.remove(other);
    }

    void erase(std::size_t ghost, std::size_t victim) {
        collectOutside({victim});

        const int line = net[ghost].line;
        for (Inside& inside : insides) {
            inside.port = Port{net.add(breed::Ghost, 0, line), 0};
        }

        rewire();
        net.remove(ghost);
        net.remove(victim);
    }

    void meetWorld(std::size_t chad, std::size_t world) {
        if (net[chad].breed == breed::Say) return say(chad, world);
        if (net[chad].breed == breed::Hear) return hear(chad, world);
        if (net[chad].breed == breed::Rep) throw errorAt(net[chad].line, "can't copy the World", "a `Rep` met the World");

        throw errorAt(net[chad].line, describe(chad) + " met the World", "only `Say` and `Hear` can");
    }

    void say(std::size_t chad, std::size_t world) {
        const std::int64_t c = net[chad].values[0];
        if (!utf8::isValidChar(c)) throw errorAt(net[chad].line, "not a character", "`Say` got " + std::to_string(c));

        io.writeBytes(utf8::encode(c));

        const Port next = net[chad].ports[1];
        net.remove(chad);
        net.link(Port{world, 0}, next);
    }

    void hear(std::size_t chad, std::size_t world) {
        const std::optional<std::int64_t> c = readCodePoint(io);

        const std::size_t got = net.add(c ? breed::Number : breed::Silence, 0, net[chad].line);
        if (c) net[got].values.push_back(*c);

        collectOutside({chad});
        insides[0].port = Port{world, 0};
        insides[1].port = Port{got, 0};

        rewire();
        net.remove(chad);
    }
};

Machine::Machine(const Program& program, Io& io) : engine(std::make_unique<Engine>(program, io)) {}

Machine::~Machine() = default;

void Machine::start() {
    engine->start();
}

Status Machine::step() {
    return engine->step();
}

Status Machine::runFor(std::uint64_t maxSteps) {
    Status status = Status::Running;
    for (std::uint64_t i = 0; i < maxSteps && status == Status::Running; i++) {
        status = engine->step();
    }

    return status;
}

std::uint64_t Machine::interactions() const {
    return engine->interactions;
}

std::uint64_t run(const Program& program, Io& io) {
    Machine machine(program, io);
    machine.start();
    machine.runFor(UINT64_MAX);

    return machine.interactions();
}

}
#include "chad/net/engine.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/lang/program.h"
#include "chad/net/expr.h"
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
    std::vector<std::size_t> dying;
    std::vector<bool> visited;
    std::vector<std::int64_t> bound;

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
        if (first == breed::Chad && second == breed::Chad) return annihilate(a, b);

        const int index = program.findRule(first, second);
        if (index < 0) throw errorAt(net[a].line, "no rule for " + describe(a) + " vs " + describe(b));

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
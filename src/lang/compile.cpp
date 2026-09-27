#include "chad/lang/compile.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/lang/lexer.h"
#include "chad/lang/parser.h"

namespace chad {

namespace {

std::string plural(std::size_t count, const char* word) {
    return (count == 0 ? std::string("no") : std::to_string(count)) + " " + word + (count == 1 ? "" : "s");
}

std::string shape(std::size_t values, std::size_t arms) {
    return plural(values, "value") + " and " + plural(arms, "arm");
}

class Compiler {
public:
    Compiler() {
        addBuiltin("number", 1, 0);
        addBuiltin("World", 0, 0);
        addBuiltin("Chad", 0, 2);
        addBuiltin("Rep", 0, 2);
        addBuiltin("Ghost", 0, 0);
        addBuiltin("Say", 1, 1);
        addBuiltin("Read", 0, 2);
        addBuiltin("Eof", 0, 0);
        addBuiltin("Cons", 1, 1);
        addBuiltin("Nil", 0, 0);
    }

    void addRules(const ast::Source& source) {
        for (const ast::Rule& rule : source.rules) {
            patternBreed(rule.left);
            patternBreed(rule.right);
            for (const ast::Case& branch : rule.cases) {
                addConnections(branch.connections);
            }
        }
    }

    void addMain(const ast::Source& source) {
        if (!source.hasMain) {
            throw errorAt(1, "no `main`", "a program starts from its `main` block");
        }
        addConnections(source.main);
    }

    Program finish() {
        const std::size_t count = program.breeds.size();
        program.ruleTable.assign(count * count, -1);
        return std::move(program);
    }

    // every use of a breed must have the same number of values and arms as the first one
    Breed breedFor(const std::string& name, std::size_t valueCount, std::size_t armCount, int line) {
        if (name == "World") {
            throw errorAt(line, "`World` belongs to the runtime", "your program gets it through the `world` wire in main");
        }

        const auto found = breedIds.find(name);
        if (found == breedIds.end()) {
            const Breed id = static_cast<Breed>(program.breeds.size());
            program.breeds.push_back({name, static_cast<int>(valueCount), static_cast<int>(armCount), line});
            breedIds.emplace(name, id);
            return id;
        }

        const BreedInfo& info = program.breeds[found->second];
        if (info.valueCount != static_cast<int>(valueCount) || info.armCount != static_cast<int>(armCount)) {
            const std::string where = info.line == 0 ? "as a built-in" : "on line " + std::to_string(info.line);
            throw errorAt(line, "wrong shape for `" + name + "`",
                          "it has " + shape(valueCount, armCount) + " here, but " +
                              shape(info.valueCount, info.armCount) + " " + where);
        }
        return found->second;
    }

private:
    Program program;
    std::map<std::string, Breed> breedIds;

    void addBuiltin(const std::string& name, int valueCount, int armCount) {
        if (name != "number") {
            breedIds.emplace(name, static_cast<Breed>(program.breeds.size()));
        }
        program.breeds.push_back({name, valueCount, armCount, 0});
    }

    Breed patternBreed(const ast::Pattern& pattern) {
        if (pattern.isNumber) return breed::Number;
        return breedFor(pattern.breed, pattern.values.size(), pattern.arms.size(), pattern.line);
    }

    void addTerm(const ast::Term& term) {
        if (term.kind != ast::TermKind::Chad) return;
        breedFor(term.name, term.values.size(), term.arms.size(), term.line);
        for (const ast::Term& arm : term.arms) {
            addTerm(arm);
        }
    }

    void addConnections(const std::vector<ast::Connection>& connections) {
        for (const ast::Connection& connection : connections) {
            addTerm(connection.left);
            addTerm(connection.right);
        }
    }
};

}

Program compile(const std::string& source) {
    Compiler compiler;
    const ast::Source program = parse(lex(source));
    compiler.addRules(program);
    compiler.addMain(program);
    return compiler.finish();
}

}

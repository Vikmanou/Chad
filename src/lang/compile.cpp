#include "chad/lang/compile.h"

#include <map>
#include <string>
#include <utility>

#include "chad/lang/lexer.h"
#include "chad/lang/parser.h"

namespace chad {

namespace {

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

    Program finish() {
        const std::size_t count = program.breeds.size();
        program.ruleTable.assign(count * count, -1);
        return std::move(program);
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
};

}

Program compile(const std::string& source) {
    Compiler compiler;
    const ast::Source program = parse(lex(source));
    return compiler.finish();
}

}

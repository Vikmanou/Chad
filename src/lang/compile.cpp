#include "chad/lang/compile.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "chad/core/error.h"
#include "chad/lang/ast.h"
#include "chad/lang/lexer.h"
#include "chad/lang/parser.h"

namespace chad {

namespace {

struct Scope {
    std::map<std::string, std::size_t> values;
    std::map<std::string, std::size_t> outside;

    std::size_t valueCount = 0;
    std::size_t outsideCount = 0;

    bool inMain = false;
};

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
            addRule(rule);
        }
    }

    void addMain(const ast::Source& source) {
        if (!source.hasMain) throw errorAt(1, "no `main`", "a program starts from its `main` block");

        Scope scope;
        scope.inMain = true;
        addConnections(source.main, scope);
    }

    Program finish() {
        const std::size_t count = program.breeds.size();
        program.ruleTable.assign(count * count, -1);
        return std::move(program);
    }

    int at(int line) const {
        return line * lineSign;
    }

    // every use of a breed must have the same number of values and arms as the first one
    Breed breedFor(const std::string& name, std::size_t valueCount, std::size_t armCount, int line) {
        if (name == "World") {
            throw errorAt(line, "`World` belongs to the runtime", "your program gets it through the `world` wire in main");
        }

        const auto found = breedIds.find(name);
        if (found == breedIds.end()) {
            const Breed id = program.breeds.size();
            program.breeds.push_back({name, valueCount, armCount, line});
            breedIds.emplace(name, id);
            return id;
        }

        const BreedInfo& info = program.breeds[found->second];
        if (info.valueCount != valueCount || info.armCount != armCount) {
            const std::string where = info.line == 0 ? "as a built-in" : "on " + lineName(info.line);
            throw errorAt(line, "wrong shape for `" + name + "`", "it has " + shape(valueCount, armCount) + " here, but " + shape(info.valueCount, info.armCount) + " " + where);
        }

        return found->second;
    }

    Code compileExpr(const ast::Expr& expr, const Scope& scope) const {
        Code code;
        code.line = at(expr.line);

        switch (expr.kind) {
            case ast::ExprKind::Number:
                code.kind = Code::Kind::Constant;
                code.constant = expr.number;
                return code;
            case ast::ExprKind::Name: {
                const auto value = scope.values.find(expr.name);
                if (value != scope.values.end()) {
                    code.kind = Code::Kind::Slot;
                    code.slot = value->second;
                    return code;
                } else if (expr.name == "_") {
                    throw errorAt(code.line, "`_` only works in rule heads");
                } else if (scope.outside.count(expr.name) != 0 || expr.name == "world") {
                    throw errorAt(code.line, "`" + expr.name + "` is a wire, not a value", "values are the names in `[ ]` or a number in the rule head");
                }
                throw errorAt(code.line, "unknown value `" + expr.name + "`");
            }
            case ast::ExprKind::Unary:
            case ast::ExprKind::Binary:
                code.kind = expr.kind == ast::ExprKind::Unary ? Code::Kind::Unary : Code::Kind::Binary;
                code.op = expr.op;
                for (const ast::Expr& operand : expr.operands) {
                    code.operands.push_back(compileExpr(operand, scope));
                }
                return code;
        }

        return code;
    }

private:
    Program program;
    std::map<std::string, Breed> breedIds;
    std::map<std::pair<Breed, Breed>, int> ruleLines; // each pair of breeds
    int lineSign = 1;

    void addBuiltin(const std::string& name, std::size_t valueCount, std::size_t armCount) {
        if (name != "number") {
            breedIds.emplace(name, program.breeds.size());
        }
        program.breeds.push_back({name, valueCount, armCount, 0});
    }

    Breed patternBreed(const ast::Pattern& pattern) {
        if (pattern.isNumber) return breed::Number;
        return breedFor(pattern.breed, pattern.values.size(), pattern.arms.size(), pattern.line);
    }

    void bind(const ast::Pattern& pattern, Scope& scope) const {
        for (const std::string& name : pattern.values) {
            if (name != "_") {
                scope.values.emplace(name, scope.valueCount);
            }
            scope.valueCount++;
        }

        for (const std::string& name : pattern.arms) {
            if (name == "_") {
                throw errorAt(at(pattern.line), "an arm can't be `_`", "every wire needs two ends; to drop something, wire it to `Ghost`");
            }
            scope.outside.emplace(name, scope.outsideCount++);
        }
    }

    void addRule(const ast::Rule& rule) {
        patternBreed(rule.left);
        patternBreed(rule.right);

        Scope scope;
        bind(rule.left, scope);
        bind(rule.right, scope);

        for (const ast::Case& branch : rule.cases) {
            if (branch.hasCondition) {
                compileExpr(branch.condition, scope);
            }
            addConnections(branch.connections, scope);
        }
    }

    void addTerm(const ast::Term& term, const Scope& scope) {
        if (term.kind == ast::TermKind::Value) {
            compileExpr(term.value, scope);
            return;
        }
        if (term.kind != ast::TermKind::Chad) return;

        breedFor(term.name, term.values.size(), term.arms.size(), term.line);
        for (const ast::Expr& value : term.values) {
            compileExpr(value, scope);
        }
        for (const ast::Term& arm : term.arms) {
            addTerm(arm, scope);
        }
    }

    void addConnections(const std::vector<ast::Connection>& connections, const Scope& scope) {
        for (const ast::Connection& connection : connections) {
            addTerm(connection.left, scope);
            addTerm(connection.right, scope);
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
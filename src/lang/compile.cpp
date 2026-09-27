#include "chad/lang/compile.h"

#include <algorithm>
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

        for (std::size_t i = 0; i < program.rules.size(); i++) {
            const Rule& rule = program.rules[i];
            program.ruleTable[rule.left * count + rule.right] = static_cast<int>(i);
            program.ruleTable[rule.right * count + rule.left] = static_cast<int>(i);
        }

        return std::move(program);
    }

    int at(int line) const {
        return line * lineSign;
    }

    // every use of a breed must have the same number of values and arms as the first one
    Breed breedFor(const std::string& name, std::size_t valueCount, std::size_t armCount, int line) {
        if (name == "World") throw errorAt(line, "`World` belongs to the runtime", "your program gets it through the `world` wire in main");

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

    std::string breedName(Breed breed) const {
        return program.breeds[breed].name;
    }

    void checkPair(const ast::Rule& rule, Breed left, Breed right) const {
        for (const Breed side : {left, right}) {
            if (side == breed::Rep) {
                throw errorAt(at(rule.line), "`Rep` rules are built in", "Rep clones whatever it faces");
            } else if (side == breed::Ghost) {
                throw errorAt(at(rule.line), "`Ghost` rules are built in", "Ghost erases whatever it faces");
            } else if (side == breed::Say || side == breed::Read || side == breed::World) {
                throw errorAt(at(rule.line), "`" + breedName(side) + "` only faces the World", "it can't have rules");
            }
        }

        if (left == breed::Number && right == breed::Number) {
            throw errorAt(at(rule.line), "a rule needs a breed", "two numbers facing off can't have a rule");
        } else if (left == breed::Chad && right == breed::Chad) {
            throw errorAt(at(rule.line), "`Chad vs Chad` is built in", "two Chads cancel out and join arms");
        } else if (left == right) {
            throw errorAt(at(rule.line), "`" + breedName(left) + "` can't have a rule with itself", "both sides would be the same breed, so the result could depend on which one is which");
        }
    }

    void addRule(const ast::Rule& rule) {
        const Breed left = patternBreed(rule.left);
        const Breed right = patternBreed(rule.right);
        checkPair(rule, left, right);

        const auto [found, isNew] = ruleLines.emplace(std::minmax(left, right), at(rule.line));
        if (!isNew) throw errorAt(at(rule.line), "second rule for `" + breedName(left) + " vs " + breedName(right) + "`", "the first one is on " + lineName(found->second));

        Scope scope;
        bind(rule.left, scope);
        bind(rule.right, scope);

        Rule compiled;
        compiled.left = left;
        compiled.right = right;
        compiled.line = at(rule.line);

        for (const ast::Case& branch : rule.cases) {
            RuleCase compiledCase;
            compiledCase.line = at(branch.line);
            if (branch.hasCondition) {
                compiledCase.hasCondition = true;
                compiledCase.condition = compileExpr(branch.condition, scope);
            }
            addConnections(branch.connections, scope);
            compiled.cases.push_back(std::move(compiledCase));
        }

        program.rules.push_back(std::move(compiled));
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
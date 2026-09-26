#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chad {

enum class Operator {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Eq,
    Ne,
    Lt,
    Gt,
    Le,
    Ge,
    And,
    Or,
    Negate,
    Not
};

}

// the program as written before names are checked
namespace chad::ast {

enum class ExprKind {
    Number,
    Name,
    Unary,
    Binary
};

// `n - 1`, `c >= '0' and c <= '9'`
struct Expr {
    ExprKind kind = ExprKind::Number;
    std::int64_t number = 0;
    std::string name;
    Operator op = Operator::Add;
    std::vector<Expr> operands;
    int line = 0;
};

enum class TermKind {
    Name, // a wire or a value turned into a number Chad
    Value, // any other value turned into a number Chad
    Chad, // Breed[values](arms)
    String, // a chain of Cons Chads
};

struct Term {
    TermKind kind = TermKind::Name;
    std::string name;
    Expr value;
    std::vector<Expr> values;
    std::vector<Term> arms;
    std::vector<std::int64_t> text;
    int line = 0;
};

// `left ~ right` (the two faces touch)
struct Connection {
    Term left;
    Term right;
    int line = 0;
};

// `Step[a, b](r)` or `n` (number Chad)
struct Pattern {
    bool isNumber = false;
    std::string breed;
    std::vector<std::string> values;
    std::vector<std::string> arms;
    int line = 0;
};

struct Case {
    bool hasCondition = false;
    Expr condition;
    std::vector<Connection> connections;
    int line = 0;
};

struct Rule {
    Pattern left;
    Pattern right;
    std::vector<Case> cases;
    int line = 0;
};

struct Source {
    std::vector<Rule> rules;
    bool hasMain = false;
    std::vector<Connection> main;
    int mainLine = 0;
};

}
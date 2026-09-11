#include "chad/lang/parser.h"

#include <string>
#include <utility>

#include "chad/core/error.h"

namespace chad {

namespace {

using namespace ast;

bool isKeyword(const std::string& word) {
    return word == "main" || word == "vs" || word == "if" || word == "else" || word == "and" || word == "or" || word == "not";
}

Expr unary(Operator op, Expr operand, int line) {
    Expr expr;
    expr.kind = ExprKind::Unary;
    expr.op = op;
    expr.line = line;
    expr.operands.push_back(std::move(operand));
    return expr;
}

Expr binary(Operator op, Expr left, Expr right, int line) {
    Expr expr;
    expr.kind = ExprKind::Binary;
    expr.op = op;
    expr.line = line;
    expr.operands.push_back(std::move(left));
    expr.operands.push_back(std::move(right));
    return expr;
}

class Parser {
public:
    explicit Parser(const TokenList& tokens) : tokens(tokens) {}

    Source run() {
        skipNewlines();
        if (!atEnd()) fail("a rule or `main`");
        return std::move(source);
    }

private:
    const TokenList& tokens;
    std::size_t pos = 0;
    Source source;

    const Token& peek() const {
        return tokens[pos];
    }

    bool atEnd() const {
        return peek().kind == TokenKind::End;
    }

    bool isSymbol(const char* symbol) const {
        return peek().kind == TokenKind::Symbol && peek().text == symbol;
    }

    bool isWord(const char* word) const {
        return peek().kind == TokenKind::Name && peek().text == word;
    }

    bool acceptSymbol(const char* symbol) {
        if (!isSymbol(symbol)) return false;
        ++pos;
        return true;
    }

    [[noreturn]] void fail(const std::string& wanted) const {
        throw errorAt(peek().line, "expected " + wanted, "got `" + peek().text + "`");
    }

    void expectSymbol(const char* symbol) {
        if (!acceptSymbol(symbol)) fail(std::string("`") + symbol + "`");
    }

    void skipNewlines() {
        while (peek().kind == TokenKind::Newline) {
            ++pos;
        }
    }

    std::string takeName(const std::string& wanted) {
        if (peek().kind != TokenKind::Name || isKeyword(peek().text)) fail(wanted);
        return tokens[pos++].text;
    }

    // lowest to highest: comparisons, + -, * / %, unary -
    Expr parseExpr() {
        return parseComparison();
    }

    Expr parseComparison() {
        Expr left = parseSum();
        const std::pair<const char*, Operator> comparisons[] = {
            {"==", Operator::Eq},
            {"!=", Operator::Ne},
            {"<", Operator::Lt},
            {">", Operator::Gt},
            {"<=", Operator::Le},
            {">=", Operator::Ge},
        };
        for (const auto& [symbol, op] : comparisons) {
            if (isSymbol(symbol)) {
                const int line = tokens[pos++].line;
                return binary(op, std::move(left), parseSum(), line);
            }
        }
        return left;
    }

    Expr parseSum() {
        Expr left = parseProduct();
        while (isSymbol("+") || isSymbol("-")) {
            const Operator op = isSymbol("+") ? Operator::Add : Operator::Sub;
            const int line = tokens[pos++].line;
            left = binary(op, std::move(left), parseProduct(), line);
        }
        return left;
    }

    Expr parseProduct() {
        Expr left = parseUnary();
        while (isSymbol("*") || isSymbol("/") || isSymbol("%")) {
            const Operator op = isSymbol("*") ? Operator::Mul : isSymbol("/") ? Operator::Div
                                                                              : Operator::Mod;
            const int line = tokens[pos++].line;
            left = binary(op, std::move(left), parseUnary(), line);
        }
        return left;
    }

    Expr parseUnary() {
        if (isSymbol("-")) {
            const int line = tokens[pos++].line;
            return unary(Operator::Negate, parseUnary(), line);
        }
        return parsePrimary();
    }

    Expr parsePrimary() {
        Expr expr;
        expr.line = peek().line;

        if (peek().kind == TokenKind::Number || peek().kind == TokenKind::Char) {
            expr.kind = ExprKind::Number;
            expr.number = tokens[pos++].value;
            return expr;
        }
        if (acceptSymbol("(")) {
            expr = parseExpr();
            expectSymbol(")");
            return expr;
        }
        expr.kind = ExprKind::Name;
        expr.name = takeName("a Chad, a wire or a value");
        return expr;
    }
};

}

ast::Source parse(const TokenList& tokens) {
    return Parser(tokens).run();
}

}
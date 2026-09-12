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
        while (!atEnd()) {
            if (isWord("main")) {
                parseMain();
            } else {
                parseRule();
            }
            skipNewlines();
        }
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

    void expectLineEnd() const {
        if (peek().kind != TokenKind::Newline && !atEnd()) fail("the end of the line");
    }

    bool lineHasVs() const {
        for (std::size_t i = pos; tokens[i].kind != TokenKind::Newline && tokens[i].kind != TokenKind::End; ++i) {
            if (tokens[i].kind == TokenKind::Name && tokens[i].text == "vs") return true;
        }
        return false;
    }

    std::string takeName(const std::string& wanted) {
        if (peek().kind != TokenKind::Name || isKeyword(peek().text)) fail(wanted);
        return tokens[pos++].text;
    }

    // main, then one or more lines of connections, until a rule starts
    void parseMain() {
        const int line = peek().line;
        ++pos;
        source.hasMain = true;
        source.mainLine = line;

        while (true) {
            skipNewlines();
            if (atEnd() || isWord("main") || lineHasVs()) break;

            for (Connection& connection : parseConnections()) {
                source.main.push_back(std::move(connection));
            }
            expectLineEnd();
        }
    }

    // Left vs Right => connections
    void parseRule() {
        Rule rule;
        rule.line = peek().line;
        rule.left = parsePattern();
        if (!isWord("vs")) fail("`vs`");
        ++pos;
        rule.right = parsePattern();

        expectSymbol("=>");
        Case single;
        single.line = rule.line;
        skipNewlines();
        single.connections = parseConnections();
        expectLineEnd();
        rule.cases.push_back(std::move(single));
        source.rules.push_back(std::move(rule));
    }

    // Breed[values](arms), or a name for a number Chad
    Pattern parsePattern() {
        Pattern pattern;
        pattern.line = peek().line;

        if (peek().kind == TokenKind::Name && !isKeyword(peek().text)) {
            pattern.isNumber = true;
            pattern.values.push_back(tokens[pos++].text);
            return pattern;
        }
        if (peek().kind != TokenKind::Breed) fail("a Chad to match, like `Fib(r)` or `n`");

        pattern.breed = tokens[pos++].text;
        if (acceptSymbol("[")) {
            do {
                pattern.values.push_back(takeName("a value name"));
            } while (acceptSymbol(","));
            expectSymbol("]");
        }
        if (acceptSymbol("(") && !acceptSymbol(")")) {
            do {
                pattern.arms.push_back(takeName("a wire name"));
            } while (acceptSymbol(","));
            expectSymbol(")");
        }
        return pattern;
    }

    // a ~ b, c ~ d
    std::vector<Connection> parseConnections() {
        std::vector<Connection> connections;
        do {
            skipNewlines();
            Connection connection;
            connection.line = peek().line;
            connection.left = parseTerm();
            expectSymbol("~");
            skipNewlines();
            connection.right = parseTerm();
            connections.push_back(std::move(connection));
        } while (acceptSymbol(","));
        return connections;
    }

    Term parseTerm() {
        Term term;
        term.line = peek().line;

        if (peek().kind == TokenKind::Breed) {
            term.kind = TermKind::Chad;
            term.name = tokens[pos++].text;
            if (acceptSymbol("[")) {
                do {
                    term.values.push_back(parseExpr());
                } while (acceptSymbol(","));
                expectSymbol("]");
            }
            if (acceptSymbol("(") && !acceptSymbol(")")) {
                do {
                    term.arms.push_back(parseTerm());
                } while (acceptSymbol(","));
                expectSymbol(")");
            }
            return term;
        }

        if (peek().kind == TokenKind::String) {
            term.kind = TermKind::String;
            term.text = tokens[pos++].chars;
            return term;
        }

        Expr value = parseExpr();
        if (value.kind == ExprKind::Name) {
            term.kind = TermKind::Name;
            term.name = value.name;
        } else {
            term.kind = TermKind::Value;
            term.value = std::move(value);
        }
        return term;
    }

    // lowest to highest: or, and, not, comparisons, + -, * / %, unary -
    Expr parseExpr() {
        Expr left = parseAnd();
        while (isWord("or")) {
            const int line = tokens[pos++].line;
            left = binary(Operator::Or, std::move(left), parseAnd(), line);
        }
        return left;
    }

    Expr parseAnd() {
        Expr left = parseNot();
        while (isWord("and")) {
            const int line = tokens[pos++].line;
            left = binary(Operator::And, std::move(left), parseNot(), line);
        }
        return left;
    }

    Expr parseNot() {
        if (isWord("not")) {
            const int line = tokens[pos++].line;
            return unary(Operator::Not, parseNot(), line);
        }
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
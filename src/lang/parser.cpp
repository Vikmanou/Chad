#include "chad/lang/parser.h"

#include <string>
#include <utility>

#include "chad/core/error.h"

namespace chad {

namespace {

using namespace ast;

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
};

}

ast::Source parse(const TokenList& tokens) {
    return Parser(tokens).run();
}

}
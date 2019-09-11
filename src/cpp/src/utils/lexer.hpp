#ifndef LEXER_HPP
#define LEXER_HPP

#include "grammar.cpp"

void to_json(json &j, const Token &p) {
    j = json{
        {"name",       p.name},
        {"value",      p.value},
        {"index",      p.index},
        {"type",       p.type},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
    };
}

#endif // LEXER_HPP

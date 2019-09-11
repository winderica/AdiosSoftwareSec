#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include "grammar.hpp"
#include "lexer.hpp"

struct Program {
    string kind;
    json body;
};

struct ProgramItem {
    string kind;
    int lineNumber;
    int lineOffset;

    ProgramItem(string kind, int ln, int lo)
        : kind(move(kind)), lineNumber(ln), lineOffset(lo) {}
};

struct IncludeStatement : ProgramItem {
    string library;

    IncludeStatement(int ln, int lo, string lib)
        : ProgramItem("IncludeStatement", ln, lo), library(move(lib)) {}
};

struct Comment : ProgramItem {
    string content;

    Comment(string kind, int ln, int lo, string content)
        : ProgramItem(move(kind), ln, lo), content(move(content)) {}
};

struct Type : ProgramItem {
    vector<string> modifiers;
    string name;
    bool isPointer;

    Type(int ln, int lo, vector<string> modifiers, string name, bool isPointer)
        : ProgramItem("Type", ln, lo), modifiers(move(modifiers)), name(move(name)), isPointer(isPointer) {}
};

struct Identifier : ProgramItem {
    string name;

    Identifier(int ln, int lo, string name)
        : ProgramItem("Identifier", ln, lo), name(move(name)) {}
};

struct PredefineStatement : ProgramItem {
    Identifier identifier;
    json arguments;
    json value;

    PredefineStatement(int ln, int lo, Identifier id, json args, json v)
        : ProgramItem("PredefineStatement", ln, lo), identifier(move(id)), arguments(move(args)), value(move(v)) {}
};

struct Declaration : ProgramItem {
    Type type;
    Identifier identifier;
    json length;

    Declaration(string kind, int ln, int lo, Type type, Identifier id, json len)
        : ProgramItem(kind.empty() ? "Declaration" : move(kind), ln, lo), type(move(type)), identifier(move(id)), length(move(len)) {}
};

struct Definition : Declaration {
    json value;

    Definition(const Declaration &dec, json v)
        : Declaration("Definition", dec.lineNumber, dec.lineOffset, dec.type, dec.identifier, dec.length), value(move(v)) {}
};

struct FunctionDeclaration : Declaration {
    json parameters;

    FunctionDeclaration(const Declaration &dec, json param)
        : Declaration("FunctionDeclaration", dec.lineNumber, dec.lineOffset, dec.type, dec.identifier, json()), parameters(move(param)) {}

protected:
    FunctionDeclaration(string kind, const Declaration &dec, json param)
        : Declaration(move(kind), dec.lineNumber, dec.lineOffset, dec.type, dec.identifier, json()), parameters(move(param)) {}
};

struct FunctionDefinition : FunctionDeclaration {
    json body;

    FunctionDefinition(const Declaration &dec, json param, json body)
        : FunctionDeclaration("FunctionDefinition", dec, move(param)), body(move(body)) {}
};

struct Literal : ProgramItem {
    json value;

    Literal(string kind, int ln, int lo, json v)
        : ProgramItem(move(kind), ln, lo), value(move(v)) {}
};

struct IndexExpression : ProgramItem {
    json array;
    json indexes;

    IndexExpression(int ln, int lo, json arr, json idx)
        : ProgramItem("IndexExpression", ln, lo), array(move(arr)), indexes(move(idx)) {}
};

struct CallExpression : ProgramItem {
    json arguments;
    json callee;

    CallExpression(int ln, int lo, json args, json callee)
        : ProgramItem("CallExpression", ln, lo), arguments(move(args)), callee(move(callee)) {}
};

struct ParenthesesExpression : ProgramItem {
    json expression;

    ParenthesesExpression(int ln, int lo, json exp)
        : ProgramItem("ParenthesesExpression", ln, lo), expression(move(exp)) {}
};

struct BinaryExpression : ProgramItem {
    string op;
    json left;
    json right;

    BinaryExpression(int ln, int lo, string op, json l, json r)
        : ProgramItem("BinaryExpression", ln, lo), op(move(op)), left(move(l)), right(move(r)) {}
};

struct PrefixExpression : ProgramItem {
    string op;
    json value;

    PrefixExpression(int ln, int lo, string op, json value)
        : ProgramItem("PrefixExpression", ln, lo), op(move(op)), value(move(value)) {}
};

struct PostfixExpression : ProgramItem {
    string op;
    json value;

    PostfixExpression(int ln, int lo, string op, json v)
        : ProgramItem("PostfixExpression", ln, lo), op(move(op)), value(move(v)) {}
};

struct CastExpression : ProgramItem {
    json type;
    json value;

    CastExpression(int ln, int lo, json t, json v)
        : ProgramItem("CastExpression", ln, lo), type(move(t)), value(move(v)) {}
};

struct BodyStatement : ProgramItem {
    json body;

    BodyStatement(string kind, int ln, int lo, json body)
        : ProgramItem(move(kind), ln, lo), body(move(body)) {}

};

struct IfStatement : BodyStatement {
    json condition;
    json elseBody;

    IfStatement(int ln, int lo, json cond, json body, json _else)
        : BodyStatement("IfStatement", ln, lo, move(body)), condition(move(cond)), elseBody(move(_else)) {}
};

struct WhileStatement : BodyStatement {
    json condition;

    WhileStatement(int ln, int lo, json cond, json body)
        : BodyStatement("WhileStatement", ln, lo, move(body)), condition(move(cond)) {}
};

struct DoWhileStatement : BodyStatement {
    json condition;

    DoWhileStatement(int ln, int lo, json cond, json body)
        : BodyStatement("DoWhileStatement", ln, lo, move(body)), condition(move(cond)) {}
};

struct ForStatement : BodyStatement {
    json init;
    json condition;
    json step;

    ForStatement(int ln, int lo, json init, json cond, json step, json body)
        : BodyStatement("ForStatement", ln, lo, move(body)), init(move(init)), condition(move(cond)), step(move(step)) {}
};

struct SwitchStatement : BodyStatement {
    json condition;

    SwitchStatement(int ln, int lo, json cond, json body)
        : BodyStatement("SwitchStatement", ln, lo, move(body)), condition(move(cond)) {}
};

struct CaseStatement : BodyStatement {
    json condition;

    CaseStatement(int ln, int lo, json cond, json body)
        : BodyStatement("CaseStatement", ln, lo, move(body)), condition(move(cond)) {}
};

struct DefaultStatement : BodyStatement {
    DefaultStatement(int ln, int lo, json body)
        : BodyStatement("DefaultStatement", ln, lo, move(body)) {}
};

struct ReturnStatement : ProgramItem {
    json value;

    ReturnStatement(int ln, int lo, json v)
        : ProgramItem("ReturnStatement", ln, lo), value(move(v)) {}
};

struct BreakStatement : ProgramItem {
    json label;

    BreakStatement(int ln, int lo, json label)
        : ProgramItem("BreakStatement", ln, lo), label(move(label)) {}
};

struct ContinueStatement : ProgramItem {
    json label;

    ContinueStatement(int ln, int lo, json label)
        : ProgramItem("ContinueStatement", ln, lo), label(move(label)) {}
};

struct ExpressionStatement : ProgramItem {
    json expression;

    ExpressionStatement(int ln, int lo, json exp)
        : ProgramItem("ExpressionStatement", ln, lo), expression(move(exp)) {}
};

void to_json(json &j, const Program &p) {
    j = json{
        {"kind", p.kind},
        {"body", p.body},
    };
}

void to_json(json &j, const IncludeStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"library",    p.library},
    };
}

void to_json(json &j, const Comment &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"content",    p.content},
    };
}

void to_json(json &j, const Type &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"name",       p.name},
        {"modifiers",  p.modifiers},
    };
}

void to_json(json &j, const Identifier &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"name",       p.name},
    };
}

void to_json(json &j, const Declaration &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"identifier", p.identifier},
        {"type",       p.type},
    };
    if (!p.length.empty()) {
        j["length"] = p.length;
    }
}

void to_json(json &j, const Definition &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"identifier", p.identifier},
        {"type",       p.type},
    };
    if (!p.length.empty()) {
        j["length"] = p.length;
    }
    if (!p.value.is_null()) {
        j["value"] = p.value;
    }
}

void to_json(json &j, const FunctionDeclaration &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"identifier", p.identifier},
        {"type",       p.type},
        {"parameters", p.parameters},
    };
}

void to_json(json &j, const FunctionDefinition &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"identifier", p.identifier},
        {"type",       p.type},
        {"parameters", p.parameters},
        {"body",       p.body},
    };
}

void to_json(json &j, const Literal &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"value",      p.value},
    };
}

void to_json(json &j, const IndexExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"array",      p.array},
        {"indexes",    p.indexes},
    };
}

void to_json(json &j, const CallExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"callee",     p.callee},
        {"arguments",  p.arguments},
    };
}

void to_json(json &j, const ParenthesesExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"expression", p.expression},
    };
}

void to_json(json &j, const BodyStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
    };
}

void to_json(json &j, const IfStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
        {"condition",  p.condition},
        {"elseBody",   p.elseBody},
    };
}

void to_json(json &j, const WhileStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
        {"condition",  p.condition},
    };
}

void to_json(json &j, const ForStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
        {"init",       p.init},
        {"condition",  p.condition},
        {"step",       p.step},
    };
}

void to_json(json &j, const SwitchStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
        {"condition",  p.condition},
    };
}

void to_json(json &j, const CaseStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
        {"condition",  p.condition},
    };
}

void to_json(json &j, const DefaultStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"body",       p.body},
    };
}

void to_json(json &j, const ReturnStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"value",      p.value},
    };
}

void to_json(json &j, const BreakStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"label",      p.label},
    };
}

void to_json(json &j, const ContinueStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"label",      p.label},
    };
}

void to_json(json &j, const ExpressionStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"expression", p.expression},
    };
}

void to_json(json &j, const PredefineStatement &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"identifier", p.identifier},
        {"arguments",  p.arguments},
        {"value",      p.value},
    };
}

void to_json(json &j, const BinaryExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"operator",   p.op},
        {"left",       p.left},
        {"right",      p.right},
    };
}

void to_json(json &j, const PrefixExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"operator",   p.op},
        {"value",      p.value},
    };
}

void to_json(json &j, const PostfixExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"operator",   p.op},
        {"value",      p.value},
    };
}

void to_json(json &j, const CastExpression &p) {
    j = json{
        {"kind",       p.kind},
        {"lineNumber", p.lineNumber},
        {"lineOffset", p.lineOffset},
        {"type",   p.type},
        {"value",      p.value},
    };
}

#endif // PARSER_H

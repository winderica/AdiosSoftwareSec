#include "parser.hpp"

class Parser : Grammar {
    vector<Token> tokens;
    Token curr;
    int index;

    Error unexpected(const string &expected) {
        return Error(
            "Line number %d offset %d : Unexpected token %s. Expect \"%s\".",
            curr.lineNumber,
            curr.lineOffset,
            (json(curr.value)).dump().c_str(),
            expected.c_str()
        );
    }

    json parseParameters() {
        json params = json::array();
        if (lookahead(")")) {
            return params;
        }
        while (curr.name == TYPE) {
            if (curr.value == "void" && tokens[index + 1].value == ")") {
                next();
            } else {
                Declaration declaration = parseDeclaration(parseType(), "ParameterDeclaration");
                params.push_back(declaration);
            }
            if (lookahead(")")) {
                return params;
            } else {
                consume(",");
            }
        }
        throw unexpected(") or ,");
    }

    json parseBody(bool shouldBeBlock = false) {
        json statements = json::array();
        if (shouldBeBlock && curr.value != "{") {
            throw unexpected("{");
        }
        if (curr.value == "{") {
            next();
            int lineNumber = curr.lineNumber;
            int lineOffset = curr.lineOffset;
            while (curr.value != "}") {
                for (const json &statement: parseStatement()) {
                    statements.push_back(statement);
                }
            }
            consume("}");
            return BodyStatement("BlockStatement", lineNumber, lineOffset, statements);
        } else {
            int lineNumber = curr.lineNumber;
            int lineOffset = curr.lineOffset;
            if (!lookahead(";")) {
                for (const json &statement: parseStatement()) {
                    statements.push_back(statement);
                }
            }
            return BodyStatement("InlineStatement", lineNumber, lineOffset, statements);
        }
    }

    json parseKeywordStatement() {
        int lineNumber = curr.lineNumber;
        int lineOffset = curr.lineOffset;
        if (lookahead("if")) {
            consume("(");
            json condition = parseExpression(")", false);
            if (condition.is_null()) {
                throw unexpected("if condition");
            }
            if (lookahead("else")) {
                throw unexpected("if body statement");
            }
            json body = parseBody();
            json elseBody;
            if (lookahead("else")) {
                elseBody = parseBody();
            }
            return IfStatement(lineNumber, lineOffset, condition, body, elseBody);
        } else if (lookahead("while")) {
            consume("(");
            json condition = parseExpression(")", false);
            if (condition.is_null()) {
                throw unexpected("while condition");
            }
            return WhileStatement(lineNumber, lineOffset, condition, parseBody());
        } else if (lookahead("do")) {
            json body = parseBody();
            consume("while");
            consume("(");
            json condition = parseExpression(")", false);
            if (condition.is_null()) {
                throw unexpected("while condition");
            }
            consume(";");
            return DoWhileStatement(lineNumber, lineOffset, condition, body);
        } else if (lookahead("for")) {
            consume("(");
            auto init = parseStatement(false);
            auto cond = parseExpression(";", false);
            auto step = parseExpression(")", true);
            auto body = parseBody();
            return ForStatement(lineNumber, lineOffset, init, cond, step, body);
        } else if (lookahead("return")) {
            return ReturnStatement(lineNumber, lineOffset, parseExpression(";", false));
        } else if (lookahead("break")) {
            return BreakStatement(lineNumber, lineOffset, parseExpression(";", false));
        } else if (lookahead("continue")) {
            return ContinueStatement(lineNumber, lineOffset, parseExpression(";", false));
        } else if (lookahead("switch")) {
            consume("(");
            auto cond = parseExpression(")", false);
            consume("{");
            json body = json::array();
            bool hasDefault = false;
            while (true) {
                int switchLineNumber = curr.lineNumber;
                int switchLineOffset = curr.lineOffset;
                if (lookahead("case")) {
                    auto caseCond = parseLiteral();
                    json caseBody = json::array();
                    consume(":");
                    bool isBlock = lookahead("{");
                    while (curr.value != "case" && curr.value != "default" && curr.value != "}") {
                        for (const json &statement: parseStatement()) {
                            caseBody.push_back(statement);
                        }
                    }
                    if (isBlock) {
                        consume("}");
                    }
                    body.push_back(CaseStatement(switchLineNumber, switchLineOffset, caseCond, caseBody));
                } else if (lookahead("default") && !hasDefault) {
                    hasDefault = true;
                    json defaultBody = json::array();
                    consume(":");
                    bool isBlock = lookahead("{");
                    while (curr.value != "case" && curr.value != "default" && curr.value != "}") {
                        for (const json &statement: parseStatement()) {
                            defaultBody.push_back(statement);
                        }
                    }
                    if (isBlock) {
                        consume("}");
                    }
                    body.push_back(DefaultStatement(switchLineNumber, switchLineOffset, defaultBody));
                } else if (lookahead("}")) {
                    break;
                } else if (hasDefault) {
                    throw unexpected("case");
                } else {
                    throw unexpected("case or default");
                }
            }
            return SwitchStatement(lineNumber, lineOffset, cond, body);
        } else {
            throw unexpected("keyword");
        }
    }

    json::array_t parseStatement(bool withKeyword = true) {
        json::array_t statements = json::array();
        if (curr.name == KEYWORD) {
            if (withKeyword) {
                statements.push_back(parseKeywordStatement());
            } else {
                throw unexpected("expression or definition");
            }
        } else if (curr.name == TYPE) {
            Type type = parseType();
            do {
                statements.push_back(parseDefinition(parseDeclaration(type)));
            } while (lookahead(","));
            consume(";");
        } else if (curr.value == "{") {
            statements.push_back(parseBody(true));
        } else {
            for (const json &expr: parseExpression(";", true)) {
                statements.push_back(ExpressionStatement(curr.lineNumber, curr.lineOffset, expr));
            }
        }
        return statements;
    }

    json parseExpression(const string &end, bool parseComma) {
        if (parseComma) {
            json expressions;
            bool hasComma;
            do {
                hasComma = false;
                json expr = parseBinary(parseUnary(), 0);
                expressions.push_back(expr);
                if (lookahead(",")) {
                    hasComma = true;
                }
            } while (hasComma);
            if (!end.empty()) {
                consume(end);
            }
            return expressions;
        } else {
            json expr = parseBinary(parseUnary(), 0);
            if (!end.empty()) {
                consume(end);
            }
            return expr;
        }
    }

    string scanBinaryOperator() {
        if (curr.name == OPERATOR) {
            return curr.value;
        }
        return "";
    }

    json parseBinary(json left, int minPrecedence) {
        string ahead = scanBinaryOperator();
        while (!ahead.empty() && infixPrecedence[ahead] >= minPrecedence) {
            string op = ahead;
            int lineNumber = curr.lineNumber;
            int lineOffset = curr.lineOffset;
            consume(op);
            auto right = parseUnary();
            if (right.is_null()) {
                throw unexpected("right value");
            }
            ahead = scanBinaryOperator();

            while (!ahead.empty() && infixPrecedence[ahead] > infixPrecedence[op]) {
                right = parseBinary(right, infixPrecedence[ahead]);
                if (right.is_null()) {
                    throw unexpected("right value");
                }
                ahead = scanBinaryOperator();
            }
            left = BinaryExpression(lineNumber, lineOffset, op, left, right);
        }
        return left;
    }

    json parseUnary() {
        int lineNumber = curr.lineNumber;
        int lineOffset = curr.lineOffset;
        for (const auto &item: prefixPrecedence) {
            if (lookahead(item.first)) {
                return PrefixExpression(lineNumber, lineOffset, item.first, parseUnary());
            }
        }
        if (curr.value == "(") {
            if (tokens[index + 1].name == TYPE) {
                next();
                Type type = parseType();
                consume(")");
                return CastExpression(lineNumber, lineOffset, type, parseUnary());
            }
        }
        json expression = parseLiteral();
        json indexes;
        while (lookahead("[")) {
            for (const json &expr: parseExpression("", true)) {
                indexes.push_back(expr);
            }
            consume("]");
        }
        if (!indexes.empty()) {
            expression = IndexExpression(lineNumber, lineOffset, expression, indexes);
        } else if (lookahead("(")) {
            if (!expression.is_null()) {
                json arguments = parseExpression(")", true);
                expression = CallExpression(lineNumber, lineOffset, arguments, expression);
            } else {
                expression = ParenthesesExpression(lineNumber, lineOffset, parseExpression("", false));
                consume(")");
            }
        } else {
            for (const auto &item: postfixPrecedence) {
                if (lookahead(item.first)) {
                    return PostfixExpression(lineNumber, lineOffset, item.first, expression);
                }
            }
        }
        return expression;
    }

    json parseLiteral() {
        int lineNumber = curr.lineNumber;
        int lineOffset = curr.lineOffset;
        if (lookahead("{")) {
            json entries = parseExpression("}", true);
            return Literal("ArrayLiteral", lineNumber, lineOffset, entries);
        } else if (curr.name == LITERAL) {
            if (curr.type == "StringLiteral") {
                string value;
                do {
                    value.append(curr.value);
                    next();
                } while (curr.type == "StringLiteral");
                Literal literal("StringLiteral", lineNumber, lineOffset, value);
                return literal;
            } else {
                Literal literal(curr.type, lineNumber, lineOffset, curr.value);
                next();
                return literal;
            }
        } else if (curr.name == IDENTIFIER) {
            return parseIdentifier();
        } else {
            return nullptr;
        }
    }

    Type parseType() {
        vector<string> modifiers;
        string name;
        bool hasModifier;
        int lineNumber = curr.lineNumber;
        int lineOffset = curr.lineOffset;
        do {
            hasModifier = false;
            if (curr.type == TYPE_MODIFIER && curr.value != "struct" && curr.value != "enum") {
                modifiers.push_back(curr.value);
                hasModifier = true;
                next();
            }
        } while (hasModifier);
        if (curr.value == "struct" || curr.value == "enum") {
            modifiers.push_back(curr.value);
            next();
            name = curr.value;
            next();
        } else {
            if (curr.type == TYPE_NAME) {
                name = curr.value;
                next();
            } else if (!modifiers.empty()) {
                name = modifiers.back();
                modifiers.pop_back();
            } else {
                throw unexpected("correct type name");
            }
        }
        bool isPointer = false;
        while (curr.value == "*") {
            isPointer = true;
            name.append("*");
            next();
        }
        return Type(lineNumber, lineOffset, modifiers, name, isPointer);
    }

    Declaration parseDeclaration(const Type &type, const string &kind = "") {
        int lineNumber = curr.lineNumber;
        int lineOffset = curr.lineOffset;
        while (curr.value == "*" && type.isPointer) {
            next();
        }
        Identifier identifier = parseIdentifier();
        json length;
        bool isArray = false;
        while (lookahead("[")) {
            isArray = true;
            if (!lookahead("]")) {
                length.push_back(parseExpression("", false));
                consume("]");
            } else {
                length.push_back(nullptr);
            }
        }
        return Declaration(kind, lineNumber, lineOffset, type, identifier, isArray ? length : json());
    }

    json parseDefinition(Declaration declaration, bool isGlobal = false) {
        if (lookahead("=")) {
            Definition definition(declaration, parseExpression("", false));
            if (!definition.length.is_null()) {
                definition.kind = "Array" + definition.kind;
            }
            if (isGlobal) {
                definition.kind = "Global" + definition.kind;
            }
            return definition;
        } else {
            if (!declaration.length.is_null()) {
                declaration.kind = "Array" + declaration.kind;
            }
            if (isGlobal) {
                declaration.kind = "Global" + declaration.kind;
            }
            return declaration;
        }
    }

    json parseFunction(const Declaration &declaration) {
        json parameters = parseParameters();
        if (lookahead(";")) {
            return FunctionDeclaration(declaration, parameters);
        } else {
            return FunctionDefinition(declaration, parameters, parseBody(true));
        }
    }

    json parseInclude() {
        int lineOffset = curr.lineOffset;
        int lineNumber = curr.lineNumber;
        next();
        if (curr.name == LITERAL && curr.type == LIBRARY_LITERAL) {
            IncludeStatement includeStatement(lineNumber, lineOffset, curr.value);
            next();
            return includeStatement;
        } else {
            throw unexpected("library name");
        }
    }

    json parsePredefine() {
        int lineOffset = curr.lineOffset;
        int lineNumber = curr.lineNumber;
        next();
        Identifier identifier = parseIdentifier();
        json::array_t arguments = json::array();
        if (lookahead("(")) {
            for (const json &expr: parseExpression(")", true)) {
                arguments.push_back(expr);
            }
        }
        if (!arguments.empty() && curr.value != "(") {
            throw unexpected("(");
        }
        return PredefineStatement(lineNumber, lineOffset, identifier, arguments, parseExpression("", false));
    }

    Identifier parseIdentifier() {
        if (curr.name != IDENTIFIER) {
            throw unexpected("Identifier");
        }
        Identifier identifier(curr.lineNumber, curr.lineOffset, curr.value);
        next();
        return identifier;
    }

    json parseComment() {
        Comment comment(curr.type, curr.lineNumber, curr.lineOffset, curr.value);
        next();
        return comment;
    }

    bool lookahead(const string &str) {
        if (curr.value == str) {
            next();
            return true;
        }
        return false;
    }

    void consume(const string &str) {
        if (curr.value != str) {
            throw unexpected(str);
        }
        next();
    }

    void next(bool withLineBreak = false) {
        index++;
        if (index >= tokens.size()) {
            return;
        }
        curr = tokens.at(index);
        if (!withLineBreak) {
            while (index < tokens.size() && curr.value == "\n") {
                next(true);
            }
        }
        while (index < tokens.size() && curr.name == COMMENT) {
            next(withLineBreak);
        }
    }

public:
    explicit Parser(vector<Token> tokens) : tokens(tokens), curr(tokens.at(0)), index(0) {}

    json parse() {
        json statements;
        while (index < tokens.size()) {
            Token token = curr;
            if (token.name == KEYWORD) {
                if (token.value == "enum") {

                } else if (token.value == "struct") {

                } else if (token.value == "typedef") {
                    next();
                    Declaration declaration = parseDeclaration(parseType(), "TypeDefinition");
                    typeNames.push_back(declaration.identifier.name);
                    consume(";");
                    statements.push_back(declaration);
                } else if (token.value == "union") {

                } else if (token.value == "#include") {
                    statements.push_back(parseInclude());
                } else if (token.value == "#define") {
                    statements.push_back(parsePredefine());
                } else {
                    throw unexpected("declaration or definition");
                }
            } else if (token.name == TYPE) {
                Type type = parseType();
                Declaration declaration = parseDeclaration(type);
                if (lookahead("(")) {
                    statements.push_back(parseFunction(declaration));
                } else {
                    statements.push_back(parseDefinition(declaration, true));
                    while (lookahead(",")) {
                        statements.push_back(parseDefinition(parseDeclaration(type), true));
                    }
                    consume(";");
                }
            } else if (token.name == SEPARATOR) {
                if (token.value == "\n") {
                    next();
                } else {
                    throw unexpected("declaration or definition");
                }
            } else if (token.name == COMMENT) {
                next();
            } else {
                throw unexpected("declaration or definition");
            }
        }

        Program program = {"Program", statements};
        return program;
    }
};

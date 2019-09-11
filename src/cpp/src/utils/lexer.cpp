#include "lexer.hpp"

Token::Token(string name, string value, string type, int lineNumber, int lineOffset, int index) :
    name(move(name)), value(move(value)), type(move(type)), lineNumber(lineNumber), lineOffset(lineOffset), index(index) {}

class Lexer : Grammar {
    string source;
    char curr{};
    int index = -1;
    int prevIndex = -1;
    int lineNumber = 1;
    int prevLineNumber = 1;
    int lineOffset = -1;
    int prevLineOffset = -1;
    vector<Token> tokens;

private:

    Error unexpected(const string &expected) {
        return Error(
            "Line number %d offset %d : Unexpected char %s. Expect \"%s\".",
            lineNumber,
            lineOffset,
            (json(string(1, curr))).dump().c_str(),
            expected.empty() ? "valid char" : expected.c_str()
        );
    }

    void pushToken(const string &name, const string &value, const string &type, bool skip = true) {
        tokens.emplace_back(name, value, type, prevLineNumber, prevLineOffset, prevIndex);
        if (skip) {
            skipSpaces();
        }
    }

    bool scanSeparator() {
        for (json &separator: separators) {
            if (lookahead(separator)) {
                pushToken(SEPARATOR, separator, SEPARATOR);
                return true;
            }
        }
        return false;
    }

    bool scanOperator() {
        for (const string &op: operators) {
            if (lookahead(op)) {
                pushToken(OPERATOR, op, OPERATOR);
                return true;
            }
        }
        return false;
    }

    bool scanType() {
        for (json &modifier: typeModifiers) {
            if (lookahead(modifier)) {
                pushToken(TYPE, modifier, TYPE_MODIFIER);
                return true;
            }
        }
        for (json &name: typeNames) {
            if (lookahead(name)) {
                pushToken(TYPE, name, TYPE_NAME);
                return true;
            }
        }
        return false;
    }

    bool scanKeyword() {
        for (json &keyword: keywords) {
            if (lookahead(keyword)) {
                pushToken(KEYWORD, keyword, KEYWORD);
                return true;
            }
        }
        return false;
    }

    void tokenizeTypedef() {
        pushToken(KEYWORD, "typedef", KEYWORD);
        if (scanType()) {
            typeModifiers.push_back(tokenizeIdentifier());
        } else {
            throw unexpected("Type");
        }
    }

    void tokenizeInclude() {
        pushToken(KEYWORD, "#include", KEYWORD);
        prevIndex = index;
        prevLineOffset = lineOffset;
        string str;
        if (curr == '<') {
            while (curr && curr != '>') {
                if (curr == '\n') {
                    throw unexpected("\" or <");
                }
                str.push_back(curr);
                next();
            }
        } else if (curr == '"') {
            do {
                if (curr == '\n') {
                    throw unexpected("\" or <");
                }
                str.push_back(curr);
                next();
            } while (curr && curr != '"');
        } else {
            throw unexpected("\" or <");
        }
        str.push_back(curr);
        next();

        pushToken(LITERAL, str, LIBRARY_LITERAL);
    }

    void tokenizePredefine() {
        pushToken(KEYWORD, "#define", KEYWORD);
    }

    void tokenizeChar() {
        next();
        string ch = string(1, curr);
        if (curr == '\\') {
            ch = tokenizeEscape();
        } else {
            next();
        }
        if (!lookahead("'")) {
            throw unexpected("single quote");
        }
        pushToken(LITERAL, "'" + ch + "'", CHAR_LITERAL);
    }

    void tokenizeString() {
        string str;
        next();
        while (curr && curr != '"') {
            if (curr == '\\') {
                str += tokenizeEscape();
            } else {
                str.push_back(curr);
                next();
            }
        }
        if (!lookahead("\"")) {
            throw unexpected("double quote");
        }
        pushToken(LITERAL, "\"" + str + "\"", STRING_LITERAL);
    }

    string tokenizeEscape() {
        index++;
        curr = source[index];
        if (curr == 'x') {
            next();
            int code = 0;

            for (int i = 0; i < 2; i++) {
                if (isHex(curr)) {
                    code = code * 16 + (int) string("0123456789abcdef").find((char) tolower(curr));
                    next();
                }
            }
            return string(1, code);
        } else if (isOct(curr)) {
            int code = 0;

            for (int i = 0; i < 3; i++) {
                if (isOct(curr)) {
                    code = code * 8 + (int) string("01234567").find((char) tolower(curr));
                    next();
                }
            }
            return string(1, code);
        } else if (escapes[curr]) {
            string escaped = "\\" + string(1, curr);
            next();
            return escaped;
        } else {
            throw unexpected("escape sequence");
        }
    }

    string tokenizeIdentifier() {
        if (!isIdentifierStart(curr)) {
            throw unexpected("Identifier");
        }
        string name = string(1, curr);
        next();
        while (curr && isIdentifierBody(curr)) {
            name.push_back(curr);
            next();
        }
        pushToken(IDENTIFIER, name, IDENTIFIER);
        return name;
    }

    void tokenizeNumber(int digits) {
        if (digits == 16 && !isHex(curr)) {
            throw unexpected("Number");
        }
        string value = string(1, curr);
        next();
        while ((curr && (digits == 16 ? isHex(curr) : isFloat(curr)))
               || (digits != 16 && tolower(curr) == 'e')
               || (curr == '-' && digits != 16 && tolower(source[index - 1]) == 'e')) {
            value.push_back(curr);
            next();
        }
        if (tolower(curr) == 'l') {
            value.push_back(curr);
            next();
        }
        if (tolower(curr) == 'u') {
            value.push_back(curr);
            next();
        }
        if (digits == 16 && curr == '.') {
            throw unexpected("hex number");
        }
        if (digits == 16) {
            value = "0x" + value;
        }
        pushToken(LITERAL, value, NUMBER_LITERAL);
    }

    bool tokenizeComment() {
        string comment;
        if (lookahead("/*", true)) {
            int _line = lineNumber;
            while (curr != '*' || source[index + 1] != '/') {
                comment.push_back(curr);
                next(true);
            }
            index += 2;
            curr = source[index];
            prevLineNumber = _line;
            pushToken(COMMENT, "/*" + comment + "*/", BLOCK_COMMENT);
            return true;
        } else if (lookahead("//", true)) {
            while (curr != '\n') {
                comment.push_back(curr);
                next(true);
            }
            pushToken(COMMENT, "//" + comment, INLINE_COMMENT);
            return true;
        } else {
            return false;
        }
    }

    bool lookahead(const string &str, bool keepBlanks = false) {
        int _index = index;
        int _offset = lineOffset;
        for (char ch: str) {
            if (curr != ch) {
                index = _index;
                lineOffset = _offset;
                curr = source[index];
                return false;
            }
            next();
        }

        if (isIdentifierStart(curr) && isIdentifier(str)) {
            index = _index;
            lineOffset = _offset;
            curr = source[index];
            return false;
        }

        return true;
    }

    void skipSpaces() {
        while (isSpace(curr)) {
            next();
        }
    }

    void next(bool ignoreLineBreak = false) {
        if (curr == '\n') {
            prevLineNumber = lineNumber;
            if (!ignoreLineBreak) {
                prevIndex = index;
                prevLineOffset = lineOffset;
                pushToken(SEPARATOR, "\n", "", false);
            }
            lineNumber++;
            lineOffset = -1;
        }
        index++;
        lineOffset++;
        curr = source[index];
    }

public:
    explicit Lexer(string src) :
        source(move(src)) {}

    explicit Lexer(string src, const vector<string> &customTypes) : source(move(src)) {
        for (string type: customTypes) {
            typeModifiers.push_back(type);
        }
    }

    vector<Token> tokenize() {
        next();
        skipSpaces();
        while (curr) {
            prevIndex = index;
            prevLineOffset = lineOffset;
            prevLineNumber = lineNumber;
            if (tokenizeComment()) {
            } else if (lookahead("#include")) {
                tokenizeInclude();
            } else if (lookahead("#define")) {
                tokenizePredefine();
            } else if (lookahead("typedef")) {
                tokenizeTypedef();
            } else if (scanType()) {
            } else if (scanKeyword()) {
            } else if (scanSeparator()) {
            } else if (scanOperator()) {
            } else if (curr == '\'') {
                tokenizeChar();
            } else if (curr == '"') {
                tokenizeString();
            } else if (lookahead("0x")) {
                tokenizeNumber(16);
            } else if (isFloat(curr)) {
                tokenizeNumber(10);
            } else if (isIdentifierStart(curr)) {
                tokenizeIdentifier();
            } else if (curr == '\0') {
                break;
            } else {
                throw unexpected("");
            }
        }
        return tokens;
    }
};

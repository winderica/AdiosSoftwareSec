#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "../lib/json.hpp"
#include <cstdarg>

using namespace std;
using json = nlohmann::json;

struct Error : exception {
    char text[1000]{};

    explicit Error(char const *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(text, sizeof text, fmt, ap);
        va_end(ap);
    }

    char const *what() const noexcept override { return text; }
};

struct Token {
    string name;
    string value;
    string type;
    int lineNumber;
    int lineOffset;
    int index;

    Token(string name, string value, string type, int lineNumber, int lineOffset, int index);
};


/**
 * Retrieve key from a map
 */
struct RetrieveKey {
    template<typename T>
    /**
     * reload "()" operator to return the first element of a key-value pair
     * @tparam T
     * @param pair
     * @return
     */
    typename T::first_type operator()(T pair) const {
        return pair.first;
    }
};

/**
 * Compare helper class
 */
struct Compare {
    /**
     * return whether the length of first string is greater than the second
     * @param first
     * @param second
     * @return
     */
    bool operator()(const string &first, const string &second) {
        return first.size() > second.size();
    }
};

/**
 * basic grammars
 */
struct Grammar {

    string IDENTIFIER = "Identifier";
    string KEYWORD = "Keyword";
    string TYPE = "Type";
    string TYPE_NAME = "TypeName";
    string TYPE_MODIFIER = "TypeModifier";
    string SEPARATOR = "Separator";
    string OPERATOR = "Operator";
    string COMMENT = "Comment";
    string INLINE_COMMENT = "InlineComment";
    string BLOCK_COMMENT = "BlockComment";
    string LITERAL = "Literal";
    string LIBRARY_LITERAL = "LibraryLiteral";
    string STRING_LITERAL = "StringLiteral";
    string CHAR_LITERAL = "CharLiteral";
    string NUMBER_LITERAL = "NumberLiteral";

    json separators = {
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        ",",
        ";",
    };

    /**
     * precedence of operators
     */
    map<string, int> infixPrecedence = {
        {"=",   1},
        {"+=",  1},
        {"-=",  1},
        {"*=",  1},
        {"/=",  1},
        {"%=",  1},
        {">>=", 1},
        {"<<=", 1},
        {"&=",  1},
        {"^=",  1},
        {"|=",  1},
        {"?",   2},
        {":",   2},
        {"||",  3},
        {"&&",  4},
        {"|",   5},
        {"^",   6},
        {"&",   7},
        {"<",   8},
        {">",   8},
        {"<=",  8},
        {">=",  8},
        {"==",  8},
        {"!=",  8},
        {">>",  9},
        {"<<",  9},
        {"+",   10},
        {"-",   10},
        {"*",   11},
        {"/",   11},
        {"%",   11},
        {".",   13},
        {"->",  13},
    };

    map<string, int> prefixPrecedence = {
        {"++",     12},
        {"--",     12},
        {"!",      12},
        {"~",      12},
        {"&",      12},
        {"*",      12},
        {"+",      12},
        {"-",      12},
        {"sizeof", 12}
    };

    map<string, int> postfixPrecedence = {
        {"++", 12},
        {"--", 12},
    };

    /**
     * operators got by anonymous function
     */
    vector<string> operators = [this]() -> vector<string> {
        map<string, int> merged = {};
        merged.insert(infixPrecedence.begin(), infixPrecedence.end());
        merged.insert(prefixPrecedence.begin(), prefixPrecedence.end());
        merged.insert(postfixPrecedence.begin(), postfixPrecedence.end());
        vector<string> operatorsList;
        transform(merged.begin(), merged.end(), back_inserter(operatorsList), RetrieveKey());
        Compare compare;
        sort(operatorsList.begin(), operatorsList.end(), compare);
        return operatorsList;
    }();

    /**
     * conversion of escapes
     */
    map<char, char> escapes = {
        {'a',  '\a'},
        {'b',  '\b'},
        {'f',  '\f'},
        {'n',  '\n'},
        {'r',  '\r'},
        {'t',  '\t'},
        {'v',  '\v'},
        {'\\', '\\'},
        {'\'', '\''},
        {'"',  '"'},
        {'?',  '\?'}
    };

    json keywords = {
        "break",
        "case",
        "continue",
        "default",
        "do",
        "else",
        "enum",
        "for",
        "goto",
        "if",
        "return",
        "switch",
        "typedef",
        "union",
        "while",
    };

    /**
     * basic types
     */
    json typeNames = {
        "char",
        "double",
        "float",
        "int",
        "long",
        "short",
        "void",
    };

    /**
     * type modifiers
     */
    json typeModifiers = {
        "auto",
        "const",
        "extern",
        "register",
        "signed",
        "static",
        "unsigned",
        "volatile",
        "long",
        "short",
        "struct",
    };

    /**
     * Whether current char is number
     * @param ch - current char
     * @return - result
     */
    static bool isNumber(char ch);

    /**
     * Whether current char is float
     * @param ch - current char
     * @return - result
     */
    static bool isFloat(char ch);

    /**
     * Whether current char is hexadecimal
     * @param ch - current char
     * @return - result
     */
    static bool isHex(char ch);

    /**
     * Whether current char is octal
     * @param ch - current char
     * @return - result
     */
    static bool isOct(char ch);

    /**
     * Whether current char is start of identifier
     * @param ch - current char
     * @return - result
     */
    static bool isIdentifierStart(char ch);

    /**
     * Whether current char is body of identifier
     * @param ch - current char
     * @return - result
     */
    static bool isIdentifierBody(char ch);

    /**
     * Whether str is identifier
     * @param str - current string
     * @return - result
     */
    static bool isIdentifier(string str);

    /**
     * Whether current char is space
     * @param ch - current char
     * @return - result
     */
    static bool isSpace(char ch);

    /**
     * Whether current char is illegal
     * @param ch - current char
     * @return - result
     */
    static bool isIllegal(char ch);
};

#endif // GRAMMAR_H

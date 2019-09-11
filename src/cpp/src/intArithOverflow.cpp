#include <regex>
#include "iostream"
class AOFinder {

    json ast;
    json result = json::array();
    json integers;
    json functions = {{"strlen", "int"}};
    json size = {
        {"short",                  1},
        {"short int",              1},
        {"signed short",           1},
        {"signed short int",       1},
        {"unsigned short",         1},
        {"unsigned short int",     1},
        {"int",                    2},
        {"signed",                 2},
        {"signed int",             2},
        {"unsigned",               2},
        {"unsigned int",           2},
        {"long",                   3},
        {"long int",               3},
        {"signed long",            3},
        {"signed long int",        3},
        {"unsigned long",          3},
        {"unsigned long int",      3},
        {"long long",              4},
        {"long long int",          4},
        {"signed long long",       4},
        {"signed long long int",   4},
        {"unsigned long long",     4},
        {"unsigned long long int", 4},
    };

    json parseNumber(json item) {
        if (item["kind"] == "NumberLiteral") {
            string value = item["value"];
            return json(
                {
                    {"isNumber", true},
                    {"value",    atoi(value.c_str())}
                }
            );
        } else if (item["kind"] == "Identifier") {
            string name = item["name"];
            if (integers[name].is_object()) {
                string leftType = integers[name]["type"];
                if (integers[name]["value"].is_string()) {
                    string value = integers[name]["value"];
                    return json(
                        {
                            {"isNumber", true},
                            {"value",    atoi(value.c_str())}
                        }
                    );
                } else {
                    return json(
                        {
                            {"isNumber", true},
                            {"value",    json()}
                        }
                    );
                }
            }
        } else if (item["kind"] == "CallExpression") {
            string name = item["callee"]["name"];
            if (!functions[name].is_null()) {
                string type = functions[name];
                if (!size[type].is_null()) {
                    return json(
                        {
                            {"isNumber", true},
                            {"value",    json()}
                        }
                    );
                }
            }
        } else if (item["kind"] == "BinaryExpression" || item["kind"] == "PrefixExpression" || item["kind"] == "PostfixExpression") {
            return json(
                {
                    {"isNumber", true},
                    {"value",    json()}
                }
            );
        }
        return json(
            {
                {"isNumber", false},
                {"value",    json()}
            }
        );
    }

    void traverse(const json &source) {
        string kind = source["kind"];
        if (kind == "Program") {
            json body = source["body"];
            for (const json &item: body) {
                traverse(item);
            }
        } else if (kind == "Type") {
        } else if (kind == "FunctionDefinition" || kind == "FunctionDeclaration") {
            string name = source["identifier"]["name"];
            string type;
            json::array_t modifiers = source["type"]["modifiers"];
            for (json &modifier: modifiers) {
                string m = modifier;
                type.append(m);
                type.append(" ");
            }
            string typeName = source["type"]["name"];
            type.append(typeName);
            functions[name] = type;
            traverse(source["type"]);
            traverse(source["identifier"]);
            json params = source["parameters"];
            for (const json &param : params) {
                traverse(param);
            }
            if (kind == "FunctionDeclaration") {
            } else {
                traverse(source["body"]);
            }
        } else if (kind == "GlobalDeclaration" || kind == "GlobalDefinition"
                   || kind == "GlobalArrayDefinition" || kind == "GlobalArrayDeclaration"
                   || kind == "ArrayDefinition" || kind == "ArrayDeclaration"
                   || kind == "Definition" || kind == "Declaration"
                   || kind == "ForVariableDefinition" || kind == "ForVariableDeclaration"
                   || kind == "ParameterDeclaration") {
            traverse(source["type"]);
            traverse(source["identifier"]);
            if (kind.find("Array") != string::npos) {
                json lengths = source["length"];
                for (const json &length: lengths) {
                    if (!length.is_null()) {
                        traverse(length);
                    }
                }
            } else {
                string type;
                json::array_t modifiers = source["type"]["modifiers"];
                for (json &modifier: modifiers) {
                    string m = modifier;
                    type.append(m);
                    type.append(" ");
                }
                string typeName = source["type"]["name"];
                type.append(typeName);
                if (size[type].is_number()) {
                    string name = source["identifier"]["name"];
                    if (kind.find("Definition") != string::npos) {
                        integers[name] = json({{"type",  type},
                                               {"value", source["value"]}});
                    } else {
                        integers[name] = json({{"type",  type},
                                               {"value", json()}});
                    }
                }
            }
            if (kind.find("Definition") != string::npos) {
                json value = source["value"];
                traverse(value);
            }
        } else if (kind.find("NumberLiteral") != string::npos) {
        } else if (kind == "CharLiteral") {
        } else if (kind == "StringLiteral") {
        } else if (kind == "ArrayLiteral") {
            json values = source["value"];
            for (const auto &value : values) {
                traverse(value);
            }
        } else if (kind == "BinaryExpression") {
            json left = source["left"];
            json right = source["right"];
            string op = source["operator"];
            if (op == "+" || op == "*") {
                json leftResult = parseNumber(left);
                json rightResult = parseNumber(right);
                if (leftResult["isNumber"] && rightResult["isNumber"]) {
                    if (leftResult["value"].is_null() || rightResult["value"].is_null()) {
                        json data = json(
                            {
                                {"operator",    op},
                                {"lineNumber",  source["lineNumber"]},
                                {"lineOffset",  source["lineOffset"]},
                                {"description", "operator " + op + " may cause overflow"}
                            }
                        );
                        result.push_back(data);
                    }
                }
//                else {
//                    json data = json(
//                        {
//                            {"operator",    op},
//                            {"lineNumber",  source["lineNumber"]},
//                            {"lineOffset",  source["lineOffset"]},
//                            {"description", "operator " + op + " may cause overflow"}
//                        }
//                    );
//                    result.push_back(data);
//                }
            }
            traverse(left);
            traverse(right);
        } else if (kind == "PrefixExpression") {
            json value = source["value"];
            string op = source["operator"];
            traverse(value);
        } else if (kind == "PostfixExpression") {
            json value = source["value"];
            string op = source["operator"];
            traverse(value);
        } else if (kind == "IndexExpression") {
            string name = source["array"]["name"];
            json indexes = source["indexes"];
            for (const json &index: indexes) {
                traverse(index);
            }
        } else if (kind == "CallExpression") {
            string name = source["callee"]["name"];
            json arguments = source["arguments"];
            for (const auto &argument : arguments) {
                if (!argument.is_null()) {
                    traverse(argument);
                }
            }
        } else if (kind == "ParenthesesExpression") {
            traverse(source["expression"]);
        } else if (kind == "Identifier") {
        } else if (kind == "ExpressionStatement") {
            json expression = source["expression"];
            if (!expression.is_null()) {
                traverse(expression);
            }
        } else if (kind == "BlockStatement" || kind == "InlineStatement") {
            json body = source["body"];
            for (const json &item: body) {
                traverse(item);
            }
        } else if (kind == "IfStatement") {
            json condition = source["condition"];
            if (!condition.is_null()) {
                traverse(condition);
            }
            traverse(source["body"]);
            json else_ = source["elseBody"];
            if (!else_.is_null()) {
                traverse(source["elseBody"]);
            }
        } else if (kind == "ForStatement") {
            json init = source["init"];
            for (const json &subInit: init) {
                traverse(subInit);
            }
            json condition = source["condition"];
            if (!condition.is_null()) {
                traverse(condition);
            }
            json step = source["step"];
            if (!step.is_null()) {
                traverse(step);
            }
            traverse(source["body"]);
        } else if (kind == "WhileStatement") {
            json condition = source["condition"];
            if (!condition.is_null()) {
                traverse(condition);
            }
            traverse(source["body"]);
        } else if (kind == "DoWhileStatement") {
            traverse(source["body"]);
            json condition = source["condition"];
            if (!condition.is_null()) {
                traverse(condition);
            }
        } else if (kind == "ReturnStatement") {
            json value = source["value"];
            if (!value.is_null()) {
                traverse(value);
            }
        } else if (kind == "BreakStatement") {
            json label = source["label"];
            if (!label.is_null()) {
                traverse(label);
            }
        } else if (kind == "ContinueStatement") {
            json label = source["label"];
            if (!label.is_null()) {
                traverse(label);
            }
        } else if (kind == "IncludeStatement") {
        } else if (kind == "PredefineStatement") {
            string identifier = source["identifier"]["name"];
            json arguments = source["arguments"];
            if (!arguments.is_null()) {
                for (const auto &argument : arguments) {
                    traverse(argument);
                }
            }
            json value = source["value"];
            traverse(value);
        } else if (kind == "TypeDefinition") {
            traverse(source["type"]);
            traverse(source["identifier"]);
        } else if (kind == "InlineComment") {
        } else if (kind == "BlockComment") {
        } else if (kind == "CastExpression") {
            json type = source["type"];
            json value = source["value"];
            traverse(type);
            if (!value.is_null()) {
                traverse(value);
            }
        }
    }

public:
    explicit AOFinder(json ast) : ast(move(ast)) {}

    json findVulnerable() {
        traverse(ast);
        return result;
    }
};

json intAO(string code, const vector<string> &types) {
    auto tokens = Lexer(move(code), types).tokenize();
    auto ast = Parser(tokens).parse();
    return AOFinder(ast).findVulnerable();
}

Napi::Value IntArithOverflow(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsString() || !info[1].IsArray()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        string code = info[0].As<Napi::String>();
        auto arr = info[1].As<Napi::Array>();
        vector<string> types;
        for (uint32_t i = 0; i < arr.Length(); i++) {
            types.push_back(arr.Get(i).As<Napi::String>());
        }
        return Napi::String::New(env, intAO(code, types).dump());
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

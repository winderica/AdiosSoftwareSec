#include <regex>

class WOFinder {

    json ast;
    json result = json::array();
    json integers;
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


    void traverse(const json &source) {
        string kind = source["kind"];
        if (kind == "Program") {
            json body = source["body"];
            for (const json &item: body) {
                traverse(item);
            }
        } else if (kind == "Type") {
        } else if (kind == "FunctionDefinition" || kind == "FunctionDeclaration") {
            traverse(source["type"]);
            traverse(source["identifier"]);
            json params = source["parameters"];
            for (auto param : params) {
                string typeName = param["type"]["name"];
                traverse(param["identifier"]);
            }
            if (kind == "FunctionDeclaration") {
            } else {
                string name = source["identifier"]["name"];
                traverse(source["body"]);
            }
        } else if (kind == "GlobalDeclaration" || kind == "GlobalDefinition"
                   || kind == "GlobalArrayDefinition" || kind == "GlobalArrayDeclaration"
                   || kind == "ArrayDefinition" || kind == "ArrayDeclaration"
                   || kind == "Definition" || kind == "Declaration"
                   || kind == "ForVariableDefinition" || kind == "ForVariableDeclaration") {
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
                    integers[name] = type;
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
            if (op == "=") {
                if (left["name"].is_string() && right["name"].is_string()) {
                    string leftName = left["name"];
                    string rightName = right["name"];
                    if (integers[leftName].is_string() && integers[rightName].is_string()) {
                        string leftType = integers[leftName];
                        string rightType = integers[rightName];
                        int leftSize = size[leftType];
                        int rightSize = size[rightType];
                        if (leftSize < rightSize) {
                            json data = json(
                                {
                                    {"left",        leftName},
                                    {"right",       rightName},
                                    {"lineNumber",  source["lineNumber"]},
                                    {"lineOffset",  source["lineOffset"]},
                                    {"description", "range of left type " + leftType + " is narrower than right type " + rightType}
                                }
                            );
                            result.push_back(data);
                        }
                    }
                }
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
                if (!subInit.is_null()) {
                    traverse(subInit);
                }
            }
            json condition = source["condition"];
            if (!condition.is_null()) {
                traverse(condition);
            }
            json step = source["step"];
            for (const json &subStep: step) {
                if (!subStep.is_null()) {
                    traverse(subStep);
                }
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
        }
    }

public:
    explicit WOFinder(json ast) : ast(move(ast)) {}

    json findVulnerable() {
        traverse(ast);
        return result;
    }
};

json intWO(string code, const vector<string>& types) {
    auto tokens = Lexer(move(code), types).tokenize();
    auto ast = Parser(tokens).parse();
    return WOFinder(ast).findVulnerable();
}

Napi::Value IntWidthOverflow(const Napi::CallbackInfo& info) {
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
        return Napi::String::New(env, intWO(code, types).dump());
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

#include "./utils/levenshtein.cpp"

class CFGGenerator {

    json functionCalls;
    json currScope;
    json ast;

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
                currScope = source;
                string name = source["identifier"]["name"];
                functionCalls[name] = json::array();
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
            string identifier = currScope["identifier"]["name"];
            functionCalls[identifier].push_back(source);
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
        }
    }

public:
    explicit CFGGenerator(json ast) : ast(move(ast)) {}

    json extract() {
        traverse(ast);
        return functionCalls;
    }
};

typedef vector <vector<string>> StringArrArr;

auto getIdentities(json cfg) {
    StringArrArr calleeArrArr;
    StringArrArr argArrArr;
    for (auto &item : cfg.items()) {
        vector <string> calleeArr;
        vector <string> argArr;
        json::array_t functions = item.value();
        for (auto function:functions) {
            calleeArr.push_back(function["callee"]["name"]);
            json::array_t arguments = function["arguments"];
            json argList;
            for (auto argument: arguments) {
                if (!argument.is_null()) {
                    string kind = argument["kind"];
                    if (kind.find("Literal") != string::npos) {
                        argList.push_back(kind);
                    } else {
                        argList.push_back("Unknown");
                    }
                }
            }
            argArr.push_back(argList.dump());
        }
        calleeArrArr.push_back(calleeArr);
        argArrArr.push_back(argArr);
    }
    return pair<StringArrArr, StringArrArr>(calleeArrArr, argArrArr);
}

double calcDistance(StringArrArr a1, StringArrArr a2) {
    int size1 = 0;
    int size2 = 0;
    for_each(a1.begin(), a1.end(), [&](const vector <string> &arr) { size1 += arr.size(); });
    for_each(a2.begin(), a2.end(), [&](const vector <string> &arr) { size2 += arr.size(); });
    double distance = 0;
    for (const auto &arr1: a1) {
        vector<int> distances;
        for (const auto &arr2: a2) {
            distances.push_back(levenshteinDistance(arr1, arr2));
        }
        distance += *min_element(distances.begin(), distances.end());
    }

    distance = 1.0 - (double) distance / max({size1, size2});
    return distance;
}

double compareCFG(string code1, string code2, const vector<string>& types) {
    auto tokens1 = Lexer(move(code1), types).tokenize();
    auto tokens2 = Lexer(move(code2), types).tokenize();
    auto ast1 = Parser(tokens1).parse();
    auto ast2 = Parser(tokens2).parse();
    auto cfg1 = CFGGenerator(ast1).extract();
    auto cfg2 = CFGGenerator(ast2).extract();
    auto identities1 = getIdentities(cfg1);
    auto identities2 = getIdentities(cfg2);
    StringArrArr calleeArrArr1 = identities1.first;
    StringArrArr argArrArr1 = identities1.second;
    StringArrArr calleeArrArr2 = identities2.first;
    StringArrArr argArrArr2 = identities2.second;
    return calcDistance(calleeArrArr1, calleeArrArr2) * 0.25 + calcDistance(argArrArr1, argArrArr2) * 0.75;
}

Napi::Value CompareCFG(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsString() || !info[1].IsString() || !info[2].IsArray()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        string code1 = info[0].As<Napi::String>();
        string code2 = info[1].As<Napi::String>();
        auto arr = info[2].As<Napi::Array>();
        vector<string> types;
        for (uint32_t i = 0; i < arr.Length(); i++) {
            types.push_back(arr.Get(i).As<Napi::String>());
        }
        return Napi::Number::New(env, compareCFG(code1, code2, types));
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

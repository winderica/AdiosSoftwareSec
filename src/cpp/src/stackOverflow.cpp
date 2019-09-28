class SOFinder {

    json arrays;
    json result = json::array();
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
                        if (length["kind"] == "NumberLiteral") {
                            string value = length["value"];
                            string name = source["identifier"]["name"];
                            int l = atoi(value.c_str());
                            arrays[name] = l;
                        }
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
            vector <string> vulnerable = {"strcpy", "strncpy", "memcpy", "strcat", "strncat", "sprintf", "vsprintf", "gets",
                                          "read", "sscanf", "fscanf", "vfscanf", "vscanf", "vsscanf"};
            if (find(vulnerable.begin(), vulnerable.end(), name) != vulnerable.end()) {
                json data = json(
                    {
                        {"function",   name},
                        {"lineNumber", source["lineNumber"]},
                        {"lineOffset", source["lineOffset"]}
                    }
                );
                if (name == "gets" || name == "read") {
                    data["description"] = name + " is always unsafe!";
                } else if (name == "strcpy" || name == "strncpy" || name == "memcpy") {
                    try {
                        unsigned long l0, l1;
                        if (source["arguments"][1]["kind"] == "StringLiteral") {
                            string value = source["arguments"][1]["value"];
                            l1 = value.length() - 2;
                        } else if (source["arguments"][1]["kind"] == "Identifier") {
                            string n = source["arguments"][1]["name"];
                            json l = arrays[n];
                            if (l.is_number()) {
                                l1 = l;
                            } else {
                                throw exception();
                            }
                        } else {
                            throw exception();
                        }
                        if (source["arguments"][0]["kind"] == "Identifier") {
                            string n = source["arguments"][0]["name"];
                            json l = arrays[n];
                            if (l.is_number()) {
                                l0 = l;
                            } else {
                                throw exception();
                            }
                        } else {
                            throw exception();
                        }
                        if (l0 < l1) {
                            data["description"] = name + " is unsafe: length " + to_string(l0) + " is less than length " + to_string(l1);
                        }
                    } catch (...) {
                        data["description"] = name + " is possibly unsafe!";
                    }
                } else {
                    data["description"] = name + " is possibly unsafe!";
                }
                result.push_back(data);
            }
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
    SOFinder(json ast) : ast(move(ast)) {}

    json findVulnerable() {
        traverse(ast);
        return result;
    }
};

json findSO(string code, const vector<string>& types) {
    auto tokens = Lexer(move(code), types).tokenize();
    auto ast = Parser(tokens).parse();
    return SOFinder(ast).findVulnerable();
}


Napi::Value StackOverflow(const Napi::CallbackInfo& info) {
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
        return Napi::String::New(env, findSO(code, types).dump());
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

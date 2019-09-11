class RCFinder {

    string assignment = "";
    json call = json();
    bool hasCheck = false;
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
                assignment = "";
                call = json();
                hasCheck = false;
                string name = source["identifier"]["name"];
                traverse(source["body"]);
                if (!call.is_null() && (assignment.empty() || (!assignment.empty() && !hasCheck))) {
                    string argument = call["name"];
                    call["description"] = "function pthread_mutex_lock on " + argument + " may cause race condition";
                    result.push_back(call);
                }
            }
        } else if (kind == "GlobalDeclaration" || kind == "GlobalDefinition"
                   || kind == "GlobalArrayDefinition" || kind == "GlobalArrayDeclaration"
                   || kind == "ArrayDefinition" || kind == "ArrayDeclaration"
                   || kind == "Definition" || kind == "Declaration"
                   || kind == "ForVariableDefinition" || kind == "ForVariableDeclaration") {
            traverse(source["type"]);
            traverse(source["identifier"]);
            string name = source["identifier"]["name"];
            if (kind.find("Global") != string::npos) {
                json data = json(
                    {
                        {"name",        name},
                        {"lineNumber",  source["lineNumber"]},
                        {"lineOffset",  source["lineOffset"]},
                        {"description", "global declaration of " + name + " may cause race condition"},
                    }
                );
                result.push_back(data);
            } else {
                json type = source["type"];
                json modifiers = type["modifiers"];
                for (const json &modifier: modifiers) {
                    if (modifier == "static") {
                        json data = json(
                            {
                                {"name",        name},
                                {"lineNumber",  source["lineNumber"]},
                                {"lineOffset",  source["lineOffset"]},
                                {"description", "static declaration of " + name + " may cause race condition"},
                            }
                        );
                        result.push_back(data);
                    }
                }
            }
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
            if (op == "=") {
                if (right["kind"] == "CallExpression" && left["kind"] == "Identifier") {
                    if (right["callee"]["name"] == "pthread_mutex_lock") {
                        if (!call.is_null() && (assignment.empty() || (!assignment.empty() && !hasCheck))) {
                            string argument = call["name"];
                            call["description"] = "function pthread_mutex_lock on " + argument + " may cause race condition";
                            result.push_back(call);
                            hasCheck = false;
                            call = json();
                        }
                        assignment = left["name"];
                    }
                }
            } else if (op == "==" || op == "!=") {
                if ((left["kind"] == "Identifier" && left["name"] == assignment)
                    || (right["kind"] == "Identifier" && right["name"] == assignment)) {
                    hasCheck = true;
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
            if (name == "pthread_mutex_lock") {
                if (!call.is_null() && (assignment.empty() || (!assignment.empty() && !hasCheck))) {
                    string argument = call["name"];
                    call["description"] = "function pthread_mutex_lock on " + argument + " may cause race condition";
                    result.push_back(call);
                    assignment = "";
                    hasCheck = false;
                }
                json argument = arguments[0];
                if (argument["kind"] == "Identifier") {
                     call = json(
                        {
                            {"name",       argument["name"]},
                            {"lineNumber", source["lineNumber"]},
                            {"lineOffset", source["lineOffset"]},
                        }
                    );
                } else if (argument["kind"] == "PrefixExpression") {
                    if (argument["value"]["kind"] == "Identifier") {
                        call = json(
                            {
                                {"name",       argument["value"]["name"]},
                                {"lineNumber", source["lineNumber"]},
                                {"lineOffset", source["lineOffset"]},
                            }
                        );
                    }
                }

            }
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
    explicit RCFinder(json ast) : ast(move(ast)) {}

    json findVulnerable() {
        traverse(ast);
        return result;
    }
};

json findRC(string code, vector<string> types) {
    types.emplace_back("pthread_mutex_t");
    auto tokens = Lexer(move(code), types).tokenize();
    auto ast = Parser(tokens).parse();
    return RCFinder(ast).findVulnerable();
}

Napi::Value RaceCondition(const Napi::CallbackInfo &info) {
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
        return Napi::String::New(env, findRC(code, types).dump());
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

#include "./utils/levenshtein.cpp"

double compareString(string code1, string code2) {
    auto tokens1 = Lexer(move(code1)).tokenize();
    auto tokens2 = Lexer(move(code2)).tokenize();
    vector<string> values1;
    vector<string> values2;
    values1.reserve(tokens1.size());
    for (const auto &token: tokens1) {
        values1.push_back(token.value);
    }
    values2.reserve(tokens2.size());
    for (const auto &token: tokens2) {
        values2.push_back(token.value);
    }
    double size = max({tokens1.size(), tokens2.size()});
    return 1.0 - levenshteinDistance(values1, values2) / size;
}

Napi::Value CompareString(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        string code1 = info[0].As<Napi::String>();
        string code2 = info[1].As<Napi::String>();
        return Napi::Number::New(env, compareString(code1, code2));
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

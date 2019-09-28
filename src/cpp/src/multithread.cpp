#include <thread>
//#include <future>
#include <ppl.h>

double wrappedCompare(string code1, string code2, const vector<string> &types) {
    try {
        return compareCFG(move(code1), move(code2), types);
    } catch (...) {
        return -1;
    }
}

vector<pair<int, double>> multithread(const vector<string> &code1Arr, const vector<string> &code2Arr, const vector<string> &types) {
    vector<pair<int, double>> result;
    concurrency::parallel_for(size_t(0), code1Arr.size(), [&](size_t i){
        result.emplace_back(pair<int, double>(i, wrappedCompare(code1Arr.at(i), code2Arr.at(i), types)));
    });
//    vector<future<double>> tasks;
//    for (int i = 0; i < code1Arr.size(); i++) {
//        auto future = async(launch::async, wrappedCompare, code1Arr.at(i), code2Arr.at(i), types);
//        tasks.push_back(move(future));
//    }
//    for (auto&& task:tasks) {
//        result.push_back(task.get());
//    }
    return result;
}

Napi::Value Multithread(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsArray() || !info[1].IsArray() || !info[2].IsArray()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        auto code1Arr = info[0].As<Napi::Array>();
        vector<string> code1s;
        for (uint32_t i = 0; i < code1Arr.Length(); i++) {
            code1s.push_back(code1Arr.Get(i).As<Napi::String>());
        }
        auto code2Arr = info[1].As<Napi::Array>();
        vector<string> code2s;
        for (uint32_t i = 0; i < code2Arr.Length(); i++) {
            code2s.push_back(code2Arr.Get(i).As<Napi::String>());
        }
        auto typesArr = info[2].As<Napi::Array>();
        vector<string> types;
        for (uint32_t i = 0; i < typesArr.Length(); i++) {
            types.push_back(typesArr.Get(i).As<Napi::String>());
        }
        auto result = Napi::Array::New(env);
        auto similarities = multithread(code1s, code2s, types);
        for (auto p : similarities) {
            result.Set<Napi::Number>(Napi::Number::New(env, p.first), Napi::Number::New(env, p.second));
        }
        return result;
    } catch (exception &e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

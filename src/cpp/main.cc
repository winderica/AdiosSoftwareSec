#include <napi.h>
#include "./src/utils/lexer.cpp"
#include "./src/utils/parser.cpp"

#include "./src/compareString.cpp"
#include "./src/compareCFG.cpp"
#include "./src/formatString.cpp"
#include "./src/heapOverflow.cpp"
#include "./src/intWidthOverflow.cpp"
#include "./src/stackOverflow.cpp"
#include "./src/intArithOverflow.cpp"
#include "./src/multithread.cpp"
#include "./src/raceCondition.cpp"

using namespace Napi;

Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "compareString"), Function::New(env, CompareString));
    exports.Set(String::New(env, "compareCFG"), Function::New(env, CompareCFG));
    exports.Set(String::New(env, "formatString"), Function::New(env, FormatString));
    exports.Set(String::New(env, "stackOverflow"), Function::New(env, StackOverflow));
    exports.Set(String::New(env, "heapOverflow"), Function::New(env, HeapOverflow));
    exports.Set(String::New(env, "intWidthOverflow"), Function::New(env, IntWidthOverflow));
    exports.Set(String::New(env, "intArithOverflow"), Function::New(env, IntArithOverflow));
    exports.Set(String::New(env, "raceCondition"), Function::New(env, RaceCondition));
    exports.Set(String::New(env, "multithread"), Function::New(env, Multithread));
    return exports;
}

NODE_API_MODULE(main, Init)

#include "gtest/gtest.h"
#include "global.hpp"

// Tiny js 
#include "externals/TinyJS.h"
#include "externals/TinyJS_Functions.h"
#include "externals/TinyJS_MathFunctions.h"

using namespace std;
using namespace CCPlus;


TEST(TinyJS, Basic) {
    const std::string code = "var result = 100;";
    CTinyJS js;

    js.execute(code);

    // Can only return a simple string
    L() << js.evaluate("result");
}

TEST(TinyJS, ReturnJSON) {
    const std::string code = "var tmlObj = {hello: 'hello world'}; var ret = JSON.stringify(tmlObj, null);";
    CTinyJS js;
    registerFunctions(&js);

    try {
        js.execute(code);
        L() << js.evaluate("ret");
    } catch(CScriptException* e) {
        L() << e->text;
    }
}

TEST(TinyJS, LoadJSON) {
    const std::string json = "{\"key\" : \"value\"}";
    const std::string code = "var tmlObj = eval(json);";
    CTinyJS js;
    registerFunctions(&js);

    js.root->addChild("json", new CScriptVar(json));

    try {
        js.execute(code);
        //L() << js.evaluate("JSON.stringify(tmlObj, null)");
        L() << js.evaluate("tmlObj.key");
    } catch(CScriptException* e) {
        L() << e->text;
    }
}

TEST(TinyJS, Loop) {
    const std::string code = "var obj = [1, 2, 3]; for (var i = 0; i < 3; i++) obj[i]++;";
    CTinyJS js;
    registerFunctions(&js);

    try {
        js.execute(code);
        L() << js.evaluate("obj[0]");
    } catch(CScriptException* e) {
        L() << e->text;
    }
}

TEST(TinyJS, TEST) {
    const std::string code = "var obj = {x: 1, y: 2};";
    CTinyJS js;
    registerFunctions(&js);

    try {
        js.execute(code);
        L() << js.evaluate("keys(obj)");
    } catch(CScriptException* e) {
        L() << e->text;
    }
}

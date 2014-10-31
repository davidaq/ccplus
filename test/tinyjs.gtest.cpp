#include "gtest/gtest.h"
#include "global.hpp"

// Tiny js 
#include "externals/TinyJS.h"
#include "externals/TinyJS_Functions.h"
#include "externals/TinyJS_MathFunctions.h"

using namespace std;
using namespace CCPlus;

CTinyJS js;
#define EVAL(x) try{js.execute(x);}catch(CScriptException *e){L()<<e->text;}

TEST(TinyJS, For) {
    std::string code;

    EVAL("var sum=0;for(var k=1;k<5;k++) {sum += k;}");
    EXPECT_EQ("10", js.evaluate("sum"));
    
    EVAL("var v={a:5,b:1,c:3};c='';var sum=0;for(var k in v) {sum+=v[k];c+=k}");
    EXPECT_EQ("9", js.evaluate("sum"));
    EXPECT_EQ("abc", js.evaluate("c"));
}

TEST(TinyJS, Basic) {
    EVAL("result = 100");
    EXPECT_EQ("100", js.evaluate("result"));
}

TEST(TinyJS, ReturnJSON) {
    CTinyJS js;
    registerFunctions(&js);
    EVAL("var tmlObj = {hello: 'hello world'}; var ret = JSON.stringify(tmlObj, null);");
    EXPECT_EQ("{ \n  \"hello\" : \"hello world\"\n}", js.evaluate("ret"));
}

TEST(TinyJS, LoadJSON) {
    const std::string json = "{\"key\" : \"value\"}";

    CTinyJS js;
    registerFunctions(&js);
    js.root->addChild("json", new CScriptVar(json));

    EVAL("var tmlObj = JSON.parse(json);");
    EXPECT_EQ("value", js.evaluate("tmlObj.key"));
}


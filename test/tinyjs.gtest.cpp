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

TEST(TinyJS, Basic) {
    EVAL("result = 100");
    EXPECT_EQ("100", js.evaluate("result"));
}

TEST(TinyJS, Increment) {
    EVAL("a = 5; b = a++; c = ++a")
    EXPECT_EQ("7", js.evaluate("a"));
    EXPECT_EQ("5", js.evaluate("b"));
    EXPECT_EQ("7", js.evaluate("c"));

    EVAL("a = 5; b = a--; c = --a");
    EXPECT_EQ("3", js.evaluate("a"));
    EXPECT_EQ("5", js.evaluate("b"));
    EXPECT_EQ("3", js.evaluate("c"));
}

TEST(TinyJS, Log) {
    CTinyJS js;
    registerFunctions(&js);
    EVAL("console.log('hello world')")
    EVAL("console.log([1,2,3,4,5], {a:2,b:3})")
}

TEST(TinyJS, For) {
    EVAL("var sum=0;for(var k=1;k<5;k++) {sum += k;}");
    EXPECT_EQ("10", js.evaluate("sum"));

    EVAL("var sum=0;for(k in [1,3,5,7,9]) {sum += k;}");
    EXPECT_EQ("10", js.evaluate("sum"));

    EVAL("var sum=0;a=[1,3,5,7,9];for(k in a) {sum += a[k];}");
    EXPECT_EQ("25", js.evaluate("sum"));
    
    EVAL("var v={a:5,b:1,c:3};c='';var sum=0;for(var k in v) {sum+=v[k];c+=k}");
    EXPECT_EQ("9", js.evaluate("sum"));
    EXPECT_EQ("abc", js.evaluate("c"));
}

TEST(TinyJS, ReturnJSON) {
    CTinyJS js;
    registerFunctions(&js);
    EVAL("var tmlObj = {hello: 'hello world'}; var ret = JSON.stringify(tmlObj);");
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

TEST(TinyJS, MapReduce) {
    CTinyJS js;
    registerFunctions(&js);

    EVAL("a=_.map([1,2,3],function(a){return a+1})");
    EXPECT_EQ("2", js.evaluate("a[0]"));
    EXPECT_EQ("3", js.evaluate("a[1]"));
    EXPECT_EQ("4", js.evaluate("a[2]"));

    EVAL("a=_.reduce([1,2,3],function(a,b){return a+b},11)");
    EXPECT_EQ("17", js.evaluate("a"));
}

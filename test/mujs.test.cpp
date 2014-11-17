#include "gtest/gtest.h"
#include "ccplus.hpp"
#include "externals/mujs/mujs.h"
#include "global.hpp"

TEST(MUJS, Basic) {
    js_State* J = js_newstate(NULL, NULL);
    js_dostring(J, "a=5*4*3*2;", 0);
    js_getglobal(J, "a");
    double a = js_tonumber(J, 0);
    L() << a;
    js_freestate(J);
}


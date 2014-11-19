#include "gtest/gtest.h"
#include "global.hpp"
extern "C" {
#include "externals/lua/lua.h"
#include "externals/lua/lualib.h"
#include "externals/lua/lauxlib.h"
}

using namespace CCPlus;
using namespace std;

TEST(Lua, Basic) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    if (luaL_dostring(L, "a = 3")) {
        lua_error(L);
        lua_close(L);
        return;
    }
    lua_getglobal(L, "a");
    int a1 = lua_tointeger(L, -1);
    EXPECT_EQ(a1, 3);

    if (luaL_dostring(L, "a = 3.1415")) {
        lua_error(L);
        lua_close(L);
        return;
    }
    lua_getglobal(L, "a");
    double a2 = lua_tonumber(L, -1);
    EXPECT_EQ(a2, 3.1415);

    if (luaL_dostring(L, "print(\" Hello world \");")) {
        lua_error(L);
        lua_close(L);
        return;
    }
    lua_close(L);
}

TEST(Lua, JSON) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dostring(L, "local json = require(\"assets/dkjson\");")) {
        lua_error(L);
        lua_close(L);
        return;
    }
    if (luaL_dofile(L, "test/res/test.lua")) {
        lua_error(L);
        lua_close(L);
        return;
    }
    lua_close(L);
}

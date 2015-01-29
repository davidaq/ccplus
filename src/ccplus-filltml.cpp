#include <fstream>
#include <sstream>
#include <regex>
#include <iterator>

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

#include "utils.hpp"
#include "ccplus.hpp"

extern "C" {
#include "externals/lua/lua.h"
#include "externals/lua/lualib.h"
#include "externals/lua/lauxlib.h"
}

int hasVolume(lua_State* L) {
    if (lua_isstring(L, 1)) {
        std::string s = lua_tostring(L, 1);
        float t = lua_tonumber(L, 2);
        float d = lua_tonumber(L, 3);
        lua_pushboolean(L, CCPlus::hasAudio(s, t, d));
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

std::string CCPlus::generateTML(const std::string& configFile, bool halfSize) {
    using boost::property_tree::ptree;
    ptree jsont;
    try {
        read_json(configFile, jsont);
    } catch (...) { 
        log(logFATAL) << "Couldn't parse or load file: " << configFile;
    }
    std::string tmlPath = jsont.get<std::string>("templateURL");
    if (tmlPath[0] != '/') { // Relative to templateURL
        tmlPath = generatePath(dirName(configFile), tmlPath);
    }

    profile (ExecutingLua) {
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);

        std::string dkjson_path = generatePath(CCPlus::assetsPath, "dkjson.lua");
        lua_pushstring(L, dkjson_path.c_str());
        lua_setglobal(L, "DKJSON_PATH");

        lua_pushstring(L, CCPlus::assetsPath.c_str());
        lua_setglobal(L, "ASSETS_PATH");

        lua_pushstring(L, slurp(tmlPath).c_str());
        lua_setglobal(L, "TPL_JSON");
        lua_pushstring(L, slurp(configFile).c_str());
        lua_setglobal(L, "USER_JSON");
        lua_pushstring(L, dirName(configFile).c_str());
        lua_setglobal(L, "USER_JSON_DIR");
        //lua_pushstring(L, readTextAsset("wrap/wrap.tml").c_str());
        lua_pushstring(L, readTextAsset("aux_tpl/aux.tml").c_str());
        lua_setglobal(L, "TPL_AUX_JSON");

        lua_pushboolean(L, JSON_BEUTIFY);
        lua_setglobal(L, "JSON_BEAUTIFY");

        lua_pushboolean(L, halfSize);
        lua_setglobal(L, "HALF_SIZE");

        lua_pushcfunction(L, hasVolume);
        lua_setglobal(L, "hasVolume");

        std::string script_path = generatePath(tmlPath, "gen_tml.lua");
        if (!file_exists(script_path)) {
            script_path = generatePath(CCPlus::assetsPath, "gen_tml.lua");
        }
        //L() << script_path;
        profile(ExecutingLuaMain) {
            if (luaL_dofile(L, script_path.c_str())) {
                lua_error(L);
                lua_close(L);
                log(logWARN) << "Failed executing script";
                return "<ERROR>";
            }
        }

        lua_getglobal(L, "RESULT");
        std::string result = lua_tostring(L, -1);
        lua_close(L);
        std::string outputPath = generatePath(dirName(configFile), "render.tml");
        spit(outputPath, result);
        return outputPath;
    }
    return "";
}


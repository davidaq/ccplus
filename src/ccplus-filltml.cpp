#include <fstream>
#include <sstream>
#include <regex>
#include <iterator>

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

#include "utils.hpp"
#include "ccplus.hpp"

#include "externals/TinyJS.h"

extern "C" {
#include "externals/lua/lua.h"
#include "externals/lua/lualib.h"
#include "externals/lua/lauxlib.h"
}

namespace boost { namespace property_tree { namespace json_parser
{
    // Create necessary escape sequences from illegal characters
    template<>
    std::basic_string<char> create_escapes(const std::basic_string<char> &s)
    {
        std::basic_string<char> result;
        std::basic_string<char>::const_iterator b = s.begin();
        std::basic_string<char>::const_iterator e = s.end();
        while (b != e)
        {
            // This assumes an ASCII superset. But so does everything in PTree.
            // We escape everything outside ASCII, because this code can't
            // handle high unicode characters.
            if (*b == 0x20 || *b == 0x21 || (*b >= 0x23 && *b <= 0x2E) ||
                (*b >= 0x30 && *b <= 0x5B) || (*b >= 0x5D && *b <= 0xFF)  //it fails here because char are signed
                || (*b >= -0x80 && *b < 0 ) ) // this will pass UTF-8 signed chars
                result += *b;
            else if (*b == char('\b')) result += char('\\'), result += char('b');
            else if (*b == char('\f')) result += char('\\'), result += char('f');
            else if (*b == char('\n')) result += char('\\'), result += char('n');
            else if (*b == char('\r')) result += char('\\'), result += char('r');
            else if (*b == char('/')) result += char('\\'), result += char('/');
            else if (*b == char('"'))  result += char('\\'), result += char('"');
            else if (*b == char('\\')) result += char('\\'), result += char('\\');
            else
            {
                const char *hexdigits = "0123456789ABCDEF";
                typedef make_unsigned<char>::type UCh;
                unsigned long u = (std::min)(static_cast<unsigned long>(
                                                 static_cast<UCh>(*b)),
                                             0xFFFFul);
                int d1 = u / 4096; u -= d1 * 4096;
                int d2 = u / 256; u -= d2 * 256;
                int d3 = u / 16; u -= d3 * 16;
                int d4 = u;
                result += char('\\'); result += char('u');
                result += char(hexdigits[d1]); result += char(hexdigits[d2]);
                result += char(hexdigits[d3]); result += char(hexdigits[d4]);
            }
            ++b;
        }
        return result;
    }
} } }

using namespace CCPlus;
using boost::property_tree::ptree;

#define ptree_array_append(ptree, X) tmpP.put("", X);ptree.push_back(std::make_pair("", tmpP))
ptree tmpP;

void CCPlus::fillTML(const std::string& jsonPath, const std::string& outputPath) {
    ptree jsont;
    try {
        read_json(jsonPath, jsont);
    } catch (...) { 
        log(logFATAL) << "Couldn't parse or load file: " << jsonPath;
    }
    std::string tmlPath = jsont.get<std::string>("templateURL");
    // relative path to json file
    if(tmlPath[0] != '/')
        tmlPath = dirName(jsonPath) + "/" + tmlPath;
    ptree tmlt;
    try {
        read_json(tmlPath, tmlt);
    } catch (...) {
        log(logFATAL) << "Couldn't parse or load file: " << tmlPath;
    }
    log(logINFO) << "Proj Path: " << jsonPath;
    log(logINFO) << "Tpl tml Path: " << tmlPath;

    auto medias = jsont.get_child("medias");
    // insert user inputs into template slot compositions
    // slot compositions are named as @1 @2 @3 .....
    for (auto& child : tmlt.get_child("compositions")) {
        std::string name = child.first.data();
        if (!stringStartsWith(name, "@")) 
            continue;

        auto& comp = child.second;


        int idx = std::atoi(name.substr(1).c_str()) - 1;
        if (idx >= medias.size()) {
            continue;
        }
        auto ite = jsont.get_child("medias").begin();
        std::advance(ite, idx);
        auto& media = (*ite).second;
        
        ptree layer;

        layer.put<std::string>("uri", "file://" + media.get<std::string>("filename"));
        float compWidth = comp.get<float>("resolution.width");
        float compHeight = comp.get<float>("resolution.height");
        float clipX = media.get<float>("x");
        float clipY = media.get<float>("y");
        float clipWidth = media.get<float>("w");
        float clipHeight = media.get<float>("h");

        ptree transform;
        // place anchor in center
        ptree_array_append(transform, compWidth / 2);
        ptree_array_append(transform, compHeight / 2);
        ptree_array_append(transform, 0);
        // put anchor to designated position
        ptree_array_append(transform, clipX + clipWidth / 2);
        ptree_array_append(transform, clipY + clipHeight / 2);
        ptree_array_append(transform, 0);
        // make right scale
        ptree_array_append(transform, compWidth / clipWidth);
        ptree_array_append(transform, compHeight / clipHeight);
        ptree_array_append(transform, 1);
        // no rotation
        ptree_array_append(transform, 0);
        ptree_array_append(transform, 0);
        ptree_array_append(transform, 0);

        float duration = comp.get<float>("duration", 1.f);
        layer.put("start", 0.f);
        layer.put("time", 0.f);
        layer.put("duration", duration);
        layer.put("last", duration);

        layer.put_child("properties.transform.0", transform);

        float volume = media.get<float>("videoVolume", 1.f);
        ptree volumes;
        ptree_array_append(volumes, volume);
        layer.put_child("properties.volume.0", volumes);

        // replace composition with the single layer
        ptree layers;
        layers.push_back(std::make_pair("", layer));
        comp.put_child("layers", layers);
    }

    // Append background musc
    try {
        std::string mainCompName = tmlt.get<std::string>("main");
        ptree& mainComp = tmlt.get_child("compositions." + mainCompName);
        float duration = mainComp.get<float>("duration");
        std::string musicPath = jsont.get<std::string>("musicURL", "");
        if (musicPath != "") {
            ptree bmusic;
            bmusic.put("uri", "file://" + musicPath);
            bmusic.put("start", 0.f);
            bmusic.put("duration", duration);
            bmusic.put("time", 0.f);
            bmusic.put("last", duration);
            bmusic.put_child("properties", ptree());
            mainComp.get_child("layers").push_back(std::make_pair("", bmusic));
        }
    } catch (...) {
    } 

    try {
        std::string mainCompName = tmlt.get<std::string>("main");
        auto ite = tmlt.get_child("compositions.@TITLE.layers").begin();
        auto& textLayer = (*ite).second;
        auto& text = textLayer.get_child("text-properties.text.0");
        text.data() = jsont.get<std::string>("videoTitle", "");
    } catch (...) {
    } 

    log(logINFO) << "Output tml Path: " << outputPath;

    std::ofstream fileStream;
    fileStream.open(outputPath);
    write_json(fileStream, tmlt, true);
    log(logINFO) << "---Successfully fill tml file! ---";
}

std::string CCPlus::generateTML(const std::string& configFile, bool halfSize) {
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
    /*
    std::string script = jsont.get<std::string>("script", "");
    if (script == "") {
        script = readTextAsset("gen_tml.js");
    }

    CTinyJS js;
    const auto& root = js.getRoot();

    root->addChild("tpljs", js.newScriptVar(slurp(tmlPath)));
    root->addChild("userjs", js.newScriptVar(slurp(configFile)));
    root->addChild("wrapjs", js.newScriptVar(readTextAsset("wrap/wrap.tml")));
    root->addChild("assetPath", js.newScriptVar(assetsPath));

    std::string result;
    try {
        profile (ExecutingJS) {
            js.execute(script);
            if(JSON_BEUTIFY) {
                if(halfSize)
                    result = js.evaluate("JSON.stringify(toHalf(result))");
                else
                    result = js.evaluate("JSON.stringify(result)");
            } else {
                if(halfSize)
                    result = js.evaluate("JSON.stringifyCompact(toHalf(result))");
                else
                    result = js.evaluate("JSON.stringifyCompact(result)");
            }
        }
    } catch (CScriptException* e) {
        L() << e->toString().c_str();
        log(logFATAL) << "Failed executing script";
        return "";
    }
    */

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
        lua_pushstring(L, readTextAsset("wrap/wrap.tml").c_str());
        lua_setglobal(L, "TPL_AUX_JSON");

        lua_pushboolean(L, JSON_BEUTIFY);
        lua_setglobal(L, "JSON_BEAUTIFY");

        lua_pushboolean(L, halfSize);
        lua_setglobal(L, "HALF_SIZE");

        std::string script_path = generatePath(CCPlus::assetsPath, "gen_tml.lua");
        if (luaL_dofile(L, script_path.c_str())) {
            lua_error(L);
            //lua_close(L);
            log(logFATAL) << "Failed executing script";
            return "";
        }

        lua_getglobal(L, "RESULT");
        std::string result = lua_tostring(L, -1);
        lua_close(L);
        std::string outputPath = generatePath(dirName(configFile), "render.tml");
        spit(outputPath, result);
        return outputPath;
    }
}


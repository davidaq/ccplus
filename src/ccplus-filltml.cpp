#include <fstream>
#include <sstream>
#include <regex>
#include <iterator>

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

#include "utils.hpp"
#include "ccplus.hpp"

using namespace CCPlus;

void CCPlus::fillTML(const std::string& jsonPath, const std::string& tmlPath, const std::string& outputPath) {
    using boost::property_tree::ptree;

    ptree jsont;
    try {
        read_json(jsonPath, jsont);
    } catch (... /* I dont care */) { 
        log(logFATAL) << "Couldn't parse or load file: " << jsonPath;
    }

    ptree tmlt;
    try {
        read_json(tmlPath, tmlt);
    } catch (...) {
        log(logFATAL) << "Couldn't parse or load file: " << tmlPath;
    }

    auto medias = jsont.get_child("medias");

    for (auto& child : tmlt.get_child("compositions")) {
        std::string name = child.first.data();
        if (!stringStartsWith(name, "__")) 
            continue;

        auto& comp = child.second;

        int idx = std::atoi(name.substr(2).c_str());
        if (idx >= medias.size()) {
            log(logFATAL) << "Couldn't find footage for " << name;
        }
        auto ite = jsont.get_child("medias").begin();
        std::advance(ite, idx);
        auto& media = (*ite).second;
        
        auto& layer = comp.get_child("layers").front().second;
        layer.put<std::string>(
                "uri", "file://" + media.get<std::string>("filename"));
        comp.put<int>(
                "resolution.width", media.get<int>("w"));
        comp.put<int>(
                "resolution.height", media.get<int>("h"));

        auto& pt0 = layer.get_child("properties.transform.0");
        ite = pt0.begin();
        std::advance(ite, 2);
        (*ite).second.put("", media.get<int>("y"));
        std::advance(ite, 1);
        (*ite).second.put("", media.get<int>("x"));
    }

    try {
        std::string mname = jsont.get<std::string>("musicURL");
        std::string maincomp = tmlt.get<std::string>("main");
        float duration = 
            tmlt.get<float>("compositions." + maincomp + ".duration");
        auto& mainCompLayers = tmlt.get_child("compositions." + maincomp + ".layers");

        // Append background musc
        ptree bmusic;
        bmusic.put("uri", "file://" + mname);
        bmusic.put<float>("start", 0.0);
        bmusic.put<float>("duration", duration);
        bmusic.put<float>("time", 0.0);
        bmusic.put<float>("last", duration);
        bmusic.put_child("properties", ptree());
        mainCompLayers.push_back(std::make_pair("", bmusic));
    } catch (...) {
        ;
    } 

    std::ostringstream ss;
    write_json(ss, tmlt, true);
    spit(outputPath, ss.str());
}

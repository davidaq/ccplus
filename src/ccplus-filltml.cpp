#include <fstream>
#include <sstream>
#include <regex>
#include <iterator>

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

#include "utils.hpp"
#include "ccplus.hpp"

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

    log(logINFO) << "Output tml Path: " << outputPath;

    std::ofstream fileStream;
    fileStream.open(outputPath);
    write_json(fileStream, tmlt, true);
    log(logINFO) << "---Successfully fill tml file! ---";
}



#include <fstream>
#include "tmlreader.hpp"
#include "context.hpp"
#include "composition.hpp"
#include "image-renderable.hpp"
#include "video-renderable.hpp"
#include "text-renderable.hpp"
#include "gif-renderable.hpp"
#include "color-renderable.hpp"

using namespace CCPlus;

Composition* TMLReader::read(const std::string& s) const {
    std::ifstream fstream(s, std::ios::in);
    if (!fstream.is_open()) {
        log(logFATAL) << "Couldn't open file: " << s;
    }
    using boost::property_tree::ptree;
    ptree pt;
    read_json(s, pt);
    fstream.close();

    std::string main_name = pt.get<std::string>("main");

    for (auto& child: pt.get_child("compositions")) {
        ptree& comp = child.second;
        initComposition(child.first.data(), comp);
    }

    return (Composition*)Context::getContext()->getRenderable("composition://" + main_name);
}

void TMLReader::initComposition(const std::string& name, const boost::property_tree::ptree& pt) const {
    Composition* comp = new Composition(
            pt.get("duration", 0.0f),
            pt.get("resolution.width", 0.0),
            pt.get("resolution.height", 0.0));

    for (auto& child: pt.get_child("layers")) {
        auto& t = child.second;
        Layer compLayer = initLayer(t, comp->width, comp->height);
        comp->appendLayer(compLayer);
    }

    Context::getContext()->putRenderable("composition://" + name, comp);
}

std::map<std::string, Property> TMLReader::readProperties(const boost::property_tree::ptree& pt) const {
    std::map<std::string, Property> ret;
    for (auto& child: pt.get_child("properties")) {
        auto name = child.first.data();
        for (auto& pc : pt.get_child(std::string("properties.") + name)) {
            float t = std::atof(pc.first.data());
            for (auto& v : pc.second) {
                float val = std::atof(v.second.data().c_str());
                ret[name][t].push_back(val);
            }
        }
    }
    return ret;
}

std::vector<std::string> TMLReader::readPropertiesOrder(const boost::property_tree::ptree& pt) const {
    std::vector<std::string> ret;
    for (auto& child: pt.get_child("properties")) {
        auto name = child.first.data();
        ret.push_back(name);
    }
    return ret;
}

std::map<std::string, std::function<Renderable*(const std::string&)> >* _extMap = 0;
Layer TMLReader::initLayer(const boost::property_tree::ptree& pt, int width, int height) const {
    std::string uri = pt.get("uri", "");
    if (!Context::getContext()->hasRenderable(uri)) {
        Renderable* renderable = 0;
        if (stringStartsWith(uri, "file://")) {
            if(!_extMap)
                _extMap = new std::map<std::string, std::function<Renderable*(const std::string&)> >();
            std::map<std::string, std::function<Renderable*(const std::string&)> >& extMap = *_extMap;
            if(extMap.empty()) {
                auto imageExt = [](const std::string& uri) {
                    return new ImageRenderable(uri);
                };
                extMap["jpg"]       = imageExt;
                extMap["png"]       = imageExt;
                extMap["bmp"]       = imageExt;
                auto gifExt = [](const std::string& uri) {
                    return new GifRenderable(uri);
                };
                extMap["gif"]       = gifExt;
                // Just treat everything else as Audio/Video
                auto avExt = [](const std::string& uri) {
                    return new VideoRenderable(uri);
                };
                extMap["default"]   = avExt;
            }
            size_t dotPos = uri.find_last_of('.');
            std::string ext = dotPos != std::string::npos ? uri.substr(dotPos + 1) : "";
            ext = toLower(ext);
            
            log(logINFO) << "Got file extention: " << ext;
            if(!extMap.count(ext)) {
                ext = "default";
                std::string path = uri;
                if (stringStartsWith(path, "file://")) 
                    path = uri.substr(7);
                path = Context::getContext()->getFootagePath(path);
                FILE* fp = fopen(path.c_str(), "rb");
                if(fp) {
                    char buff[4];
                    fread(buff, 1, 3, fp);
                    fclose(fp);
                    buff[3] = 0;
                    if(strcmp(buff, "GIF") == 0) {
                        ext = "gif";
                    }
                }
            }
            renderable = extMap[ext](uri);
        } else if (stringStartsWith(uri, "text://")) {
            renderable = new TextRenderable(pt);
        } else if (stringStartsWith(uri, "color://")) {
            renderable = new ColorRenderable(uri);
        } else if (!stringStartsWith(uri, "composition://")) {
            log(logWARN) << "Ignore unkwown footage type " << uri;
        }
        if(renderable) {
            Context::getContext()->retain(renderable);
            Context::getContext()->putRenderable(uri, renderable);
        }
    }
    int blendMode = pt.get("blend", 0);
    int trkMat = pt.get("trkMat", 0);
    bool showup = pt.get("visible", true);
    Layer l = Layer(
            uri, pt.get("time", 0.0f), pt.get("duration", 0.0f),
            pt.get("start", 0.0f), pt.get("last", 0.0f), width, height,
            blendMode, trkMat, showup);
    l.setProperties(readProperties(pt), readPropertiesOrder(pt));
    return l;
}


#include <fstream>
#include <locale>

#include "tmlreader.hpp"
#include "utils.hpp"
#include "image-renderable.hpp"
#include "video-renderable.hpp"
#include "text-renderable.hpp"
#include "gif-renderable.hpp"

using namespace CCPlus;

void fillTextProperties(TextRenderable*, 
        const boost::property_tree::ptree&); 

TMLReader::TMLReader(CCPlus::Context* ctx) :
    context(ctx)
{
}

Composition* TMLReader::read(const std::string& s) const {

    std::ifstream fstream(s, std::ios::in);
    if (!fstream.is_open()) {
        log(logFATAL) << "Couldn't open file: " << s;
    }

    context->setInputDir(dirName(s));

    using boost::property_tree::ptree;
    ptree pt;
    read_json(s, pt);
    fstream.close();

    std::string main_name = pt.get<std::string>("main");

    for (auto& child: pt.get_child("compositions")) {
        ptree& comp = child.second;

        initComposition(child.first.data(), comp);
    }

    return (Composition*)context->getRenderable("composition://" + main_name);
}

void TMLReader::initComposition(const std::string& name, const boost::property_tree::ptree& pt) const {
    Composition* comp = new Composition(
            context, 
            name,
            pt.get("duration", 0.0f),
            pt.get("resolution.width", 0.0),
            pt.get("resolution.height", 0.0));
    context->retain(comp);

    for (auto& child: pt.get_child("layers")) {
        auto& t = child.second;

        comp->putLayer(initLayer(t, comp->getWidth(), comp->getHeight()));
    }

    context->putRenderable("composition://" + comp->getName(), comp);
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

Layer TMLReader::initLayer(const boost::property_tree::ptree& pt, int width, int height) const {
    std::string uri = pt.get("uri", "");
    if (!context->hasRenderable(uri)) {
        Renderable* renderable = 0;
        if (stringStartsWith(uri, "file://")) {
            static std::map<std::string, std::function<Renderable*(Context*, const std::string&)> > extMap;
            if(extMap.empty()) {
                auto imageExt = [](Context* context, const std::string& uri) {
                    return new ImageRenderable(context, uri);
                };
                extMap["jpg"]       = imageExt;
                extMap["png"]       = imageExt;
                extMap["bmp"]       = imageExt;
                auto gifExt = [](Context* context, const std::string& uri) {
                    return new GifRenderable(context, uri);
                };
                extMap["gif"]       = gifExt;
                // Just treat everything else as Audio/Video
                auto avExt = [](Context* context, const std::string& uri) {
                    return new VideoRenderable(context, uri);
                };
                extMap["default"]   = avExt;
            }
            size_t dotPos = uri.find_last_of('.');
            std::string ext = dotPos != std::string::npos ? uri.substr(dotPos + 1) : "";
            ext = toLower(ext);
            
            log(logDEBUG) << "Got file extention: " << ext;
            if(!extMap.count(ext)) {
                ext = "default";
                std::string path = uri;
                if (stringStartsWith(path, "file://")) 
                    path = uri.substr(7);
                path = generatePath(context->getInputDir(), path);
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
            renderable = extMap[ext](context, uri);
        } else if (stringStartsWith(uri, "text://")) {
            renderable = new TextRenderable(context, uri);
            fillTextProperties((TextRenderable*)renderable, pt);
        } else if (!stringStartsWith(uri, "composition://")) {
            log(logWARN) << "Unkwown footage type " << uri;
            renderable = new ImageRenderable(context, "file://UNKNOWN");
        }
        if(renderable) {
            context->retain(renderable);
            context->putRenderable(uri, renderable);
        }
    }
    int blendMode = pt.get("blend", 0);
    int trkMat = pt.get("trkMat", 0);
    bool showup = pt.get("visible", true);
    Layer l = Layer(
            context, uri, pt.get("time", 0.0f), pt.get("duration", 0.0f),
            pt.get("start", 0.0f), pt.get("last", 0.0f), width, height,
            blendMode, trkMat, showup);
    l.setProperties(readProperties(pt), readPropertiesOrder(pt));
    return l;
}

void fillTextProperties(TextRenderable* r, 
        const boost::property_tree::ptree& tree) {
    auto each = [&tree] (const std::string& name, auto f) {
        for (auto& pc : 
                tree.get_child(std::string("text-properties.") + name)) {
            float t = std::atof(pc.first.data());
            f(t, pc.second);
        }
    };
    // I hate language without reflection
    each("text", [r] (float t, const auto& pc) {
        utf8toWStr(r->text[t], pc.data());
    });
    each("font", [&] (float t, auto& pc) {
        utf8toWStr(r->font[t], pc.data());
    });
    each("size", [&] (float t, auto& pc) {
        r->size[t] = std::atoi(pc.data().c_str());
    });
    each("tracking", [&] (float t, auto& pc) {
        r->tracking[t] = std::atof(pc.data().c_str());
    });
    each("bold", [&] (float t, auto& pc) {
        r->bold[t] = (pc.data()[0] == 't');
    });
    each("italic", [&] (float t, auto& pc) {
        r->italic[t] = (pc.data()[0] == 't');
    });
    each("scale_x", [&] (float t, auto& pc) {
        r->scale_x[t] = std::atof(pc.data().c_str());
    });
    each("scale_y", [&] (float t, auto& pc) {
        r->scale_y[t] = std::atof(pc.data().c_str());
    });
    each("color", [&] (float t, auto& pc) {
        r->color[t] = std::atoi(pc.data().c_str());
    });
    each("justification", [&] (float t, auto& pc) {
        r->justification[t] = std::atoi(pc.data().c_str());
    });
}

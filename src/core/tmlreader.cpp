#include <fstream>

#include "tmlreader.hpp"
#include "utils.hpp"
#include "image-renderable.hpp"

using namespace CCPlus;

TMLReader::TMLReader(CCPlus::Context* ctx) :
    context(ctx)
{

}

Composition* TMLReader::read(const std::string& s) const {

    std::ifstream fstream(s, std::ios::in);

    using boost::property_tree::ptree;
    ptree pt;
    printf("reading json\n");
    read_json(s, pt);
    printf("read json done\n");
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
                float val = std::stof(v.second.data());
                ret[name][t].push_back(val);
            }
        }
    }
    return ret;
}

Layer TMLReader::initLayer(const boost::property_tree::ptree& pt, int width, int height) const {
    std::string uri = pt.get("uri", "");
    if (!context->hasRenderable(uri)) {
        if (stringStartsWith(uri, "image://")) {
            context->putRenderable(uri, new ImageRenderable(context, uri));
        } else if (stringStartsWith(uri, "video://")) {

        } else {
            // What the f
        }
    }
    Layer l = Layer(
            context, uri, pt.get("time", 0.0f), pt.get("duration", 0.0f),
            pt.get("start", 0.0f), pt.get("last", 0.0f), width, height);
    l.setProperties(readProperties(pt));
    return l;
}

#include <fstream>

#include "tmlreader.hpp"

using namespace CCPlus;

TMLReader::TMLReader(CCPlus::Context* ctx) :
    context(ctx)
{

}

Composition* TMLReader::read(const std::string& s) const {

    std::ifstream fstream(s, std::ios::in);

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

    for (auto& child: pt.get_child("layers")) {
        auto& t = child.second;

        comp->putLayer(initLayer(t));
    }

    context->putRenderable("composition://" + comp->getName(), comp);
}

Layer TMLReader::initLayer(const boost::property_tree::ptree& pt) const {
    return Layer(
            new Renderable(0, 0, 0), pt.get("time", 0.0f), pt.get("duration", 0.0f),
            pt.get("start", 0.0f), pt.get("last", 0.0f));
}

#include <fstream>

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

#include "composition.hpp"
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
    std::string main_name = pt.get<std::string>("main");

    for (auto& child: pt.get_child("compositions")) {
        ptree& comp = child.second;
        Composition* new_comp = new Composition(
                context, 
                child.first.data(), 
                comp.get("duration", 0.0f),
                comp.get("resolution.width", 0.0),
                comp.get("resolution.height", 0.0));

        context->putRenderable("composition://" + new_comp->getName(), new_comp);
    }

    //std::cout << pt.get<std::string>("main") << std::endl;

    return (Composition*)context->getRenderable("composition://" + main_name);
}

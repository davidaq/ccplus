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
    std::string main_name = pt.get<std::string>("name");
    
    for (auto& child: pt.get_child("compositions")) {
        auto& comp = child.second;
        Composition* new_comp = new Composition(
                context, 
                comp.get<std::string>("name"), 
                comp.get("version", 0.0f),
                comp.get("duration", 0.0f),
                comp.get_child("resolution").front().second.get("width", 0),
                comp.get_child("resolution").back().second.get("height", 0));

        context->putRenderable("composition://" + new_comp->getName(), new_comp);
    }

    //std::cout << pt.get<std::string>("main") << std::endl;

    return (Composition*)context->getRenderable(main_name);
}

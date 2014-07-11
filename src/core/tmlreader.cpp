#include "tmlreader.hpp"
#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 
#include "composition.hpp"

using namespace CCPlus;

TMLReader::TMLReader(CCPlus::Context& ctx) :
    context(&ctx)
{

}

Composition TMLReader::read(const std::string& s) const {
    return Composition(context);
}

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

Composition TMLReader::read(const std::string& s) const {

    std::ifstream fstream(s, std::ios::in);

    using boost::property_tree::ptree;
    ptree pt;
    read_json(s, pt);

    //std::cout << pt.get<std::string>("main") << std::endl;

    return Composition(context);
}

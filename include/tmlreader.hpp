#pragma once

#include "global.hpp"

#ifdef IN_CCPLUS_PRIVATE_CONTEXT
#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 
#endif

class CCPlus::TMLReader : public CCPlus::Object {

public:
    explicit TMLReader(CCPlus::Context* context); 

    // operation
    Composition* read(const std::string& s) const; 
    
private:
    //using boost::property_tree::ptree;
#ifdef IN_CCPLUS_PRIVATE_CONTEXT
    void initComposition(const std::string&, const boost::property_tree::ptree&) const; 
    Layer initLayer(const boost::property_tree::ptree&) const;
    std::map<std::string, Property> readProperties(const boost::property_tree::ptree&) const;
#endif
    
    // data
    CCPlus::Context* context;
    
};

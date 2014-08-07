#pragma once

#include "global.hpp"

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

class CCPlus::TMLReader : public CCPlus::Object {

public:
    explicit TMLReader(CCPlus::Context* context); 

    // operation
    Composition* read(const std::string& s) const; 
    
private:
    //using boost::property_tree::ptree;
    void initComposition(const std::string&, const boost::property_tree::ptree&) const; 
    Layer initLayer(const boost::property_tree::ptree&, int width, int height) const;
    std::map<std::string, Property> readProperties(const boost::property_tree::ptree&) const;
    std::vector<std::string> readPropertiesOrder(const boost::property_tree::ptree&) const;
    
    // data
    CCPlus::Context* context;
    
};

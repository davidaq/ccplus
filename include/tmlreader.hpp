#pragma once
#include "global.hpp"
#include "layer.hpp"
#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

class CCPlus::TMLReader : public CCPlus::Object {

public:
    // operation
    Composition* read(const std::string& s) const; 
    
private:
    //using boost::property_tree::ptree;
    bool initComposition(const std::string&, const boost::property_tree::ptree&) const; 
    Layer initLayer(const boost::property_tree::ptree&, int width, int height) const;
    std::map<std::string, Property> readProperties(const boost::property_tree::ptree&) const;
};

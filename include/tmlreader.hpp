#pragma once
#include "global.hpp"
#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

struct DoubleLess {
    bool operator() (float left, float right) const
    {
        //return left < right;
        return (std::abs(left - right) > 0.00001) && (left < right);
    }
}; 
typedef std::map<float, std::vector<float>, DoubleLess> Property;
typedef std::map<std::string, Property> PropertyMap;

class CCPlus::TMLReader : public CCPlus::Object {

public:
    // operation
    Composition* read(const std::string& s) const; 
    
private:
    //using boost::property_tree::ptree;
    void initComposition(const std::string&, const boost::property_tree::ptree&) const; 
    Layer initLayer(const boost::property_tree::ptree&, int width, int height) const;
    std::map<std::string, Property> readProperties(const boost::property_tree::ptree&) const;
    std::vector<std::string> readPropertiesOrder(const boost::property_tree::ptree&) const;
};

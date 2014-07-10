#pragma once
#include <sstream>
#include <boost/property_tree/json_parser.hpp> 
#include <string>

#include "composition.hpp"

class TMLReader {

public:
    TMLReader(); 

    static Composition read(const std::string s); 
};

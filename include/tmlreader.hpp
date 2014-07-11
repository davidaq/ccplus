#pragma once
#include <sstream>

#include "global.hpp"

class CCPlus::TMLReader {

public:
    TMLReader(); 

    static Composition read(const std::string s); 
};

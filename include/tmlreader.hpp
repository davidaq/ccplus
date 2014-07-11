#pragma once
#include <sstream>
#include <string>

#include "ccplus.hpp"

class CCPlus::TMLReader {

public:
    TMLReader(); 

    static Composition read(const std::string s); 
};

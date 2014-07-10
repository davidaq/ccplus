#pragma once
#include <sstream>
#include <string>

#include "composition.hpp"

class TMLReader {

public:
    TMLReader(); 

    static Composition read(const std::string s); 
};

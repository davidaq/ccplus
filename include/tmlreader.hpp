#pragma once

#include "global.hpp"

class CCPlus::TMLReader : public CCPlus::Object {

public:
    explicit TMLReader(CCPlus::Context* context); 

    // operation
    Composition* read(const std::string& s) const; 
    
    // 
    
private:
    
    // data
    CCPlus::Context* context;
    
};

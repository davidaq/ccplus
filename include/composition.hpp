#pragma once
#include <string>
#include "ccplus.hpp"

class CCPlus::Composition {

public:
    Composition();
    
    std::string getName() const; 

    float getVersion() const;

    float getDuration() const;

    float getWidth() const;

    float getHeight() const;

};

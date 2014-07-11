#pragma once
#include "global.hpp"

class CCPlus::Composition {

public:
    Composition();
    
    std::string getName() const; 

    float getVersion() const;

    float getDuration() const;

    float getWidth() const;

    float getHeight() const;

    std::vector<Composition> dependency(float, float) const;

    std::vector<Layer> getLayers() const;

};

#pragma once
#include "global.hpp"

class CCPlus::Composition {

public:
    Composition();
    explicit Composition(CCPlus::Context* context);
    
    // access
    std::string getName() const; 

    float getVersion() const;

    float getDuration() const;

    float getWidth() const;

    float getHeight() const;

private:
    // data
    CCPlus::Context* context;
    std::vector<Composition> dependency(float, float) const;

    std::vector<Layer> getLayers() const;

};

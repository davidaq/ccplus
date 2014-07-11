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
    CCPlus::Context* context = 0;
    std::vector<Composition> dependency(float, float) const;

    std::vector<Layer> getLayers() const;

    std::string name = "";
    float version = 0;
    float duration = 0;
    float width = 0;
    float height = 0;

};

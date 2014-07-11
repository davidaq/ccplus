#pragma once
#include "global.hpp"

class CCPlus::CompositionDependency {
public:
    Renderable* renderable;
    float start, duration;
};

class CCPlus::Composition : public CCPlus::Renderable {

public:
    explicit Composition(
            CCPlus::Context* context,
            std::string, float, float, float);
    
    // access
    std::string getName() const; 

    void putLayer(const Layer&);
    std::vector<Layer> getLayers() const;

private:
    // data
    CCPlus::Context* context = 0;
    std::vector<CompositionDependency> dependency(float from, float to) const;

    std::string name = "";

    std::vector<Layer> layers;
};

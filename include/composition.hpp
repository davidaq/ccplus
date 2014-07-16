#pragma once
#include "global.hpp"

class CCPlus::CompositionDependency {
public:
    Renderable* renderable;
    float from, to;
};

class CCPlus::Composition : public CCPlus::Renderable {

public:
    explicit Composition(
            CCPlus::Context* context,
            std::string, float, float, float);

    void render(float start, float duration);
    //Image getFrame(float time);
    
    // access
    std::string getName() const; 

    void putLayer(const Layer&);
    std::vector<Layer> getLayers() const;
    
    std::vector<CompositionDependency> directDependency(float from, float to) const;
    std::vector<CompositionDependency> fullOrderedDependency(float from, float to) const;

private:
    // data
    std::string name = "";

    std::vector<Layer> layers;
};

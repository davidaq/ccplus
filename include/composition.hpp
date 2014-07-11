#pragma once
#include "global.hpp"

class CCPlus::CompositionDependency {
public:
    Renderable* renderable;
    float from, to;
};

class CCPlus::Composition : public CCPlus::Renderable {

public:
    Composition();
    explicit Composition(
            CCPlus::Context* context,
            std::string, float, float, float);
    
    // access
    std::string getName() const; 

    float getDuration() const;

    float getWidth() const;

    float getHeight() const;

private:
    // data
    CCPlus::Context* context = 0;
    std::vector<CompositionDependency> directDependency(float from, float to) const;
    std::vector<CompositionDependency> fullOrderedDependency(float from, float to) const;
    
    std::vector<Layer> getLayers() const;

    std::string name = "";
    float duration = 0;
    float width = 0;
    float height = 0;

};

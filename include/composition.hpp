#pragma once
#include "animated-renderable.hpp"
#include "layer.hpp"

namespace CCPlus {
    class CompositionDependency;
    class Composition;
}

class CCPlus::CompositionDependency {
public:
    Renderable* renderable;
    float from, to;
};

class CCPlus::Composition : public CCPlus::AnimatedRenderable {

public:
    Composition( CCPlus::Context* context,
            std::string, float, int, int);
    ~Composition();

    void renderPart(float start, float duration);
    
    // access
    std::string getName() const; 

    void putLayer(const Layer&);
    std::vector<Layer> getLayers() const;
    
    std::vector<CompositionDependency> directDependency(float from, float to) const;
    std::vector<CompositionDependency> fullOrderedDependency(float from, float to) const;

    int getWidth() const;
    int getHeight() const;
    float getDuration() const;

private:
    // data
    std::string name = "";

    std::vector<Layer> layers;

    int width, height;
    float duration;
};

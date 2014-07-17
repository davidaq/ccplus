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
            std::string, float, int, int);

    void render(float start, float duration);
    Image getFrame(float time) const;
    
    // access
    std::string getName() const; 

    void putLayer(const Layer&);
    std::vector<Layer> getLayers() const;
    
    std::vector<CompositionDependency> directDependency(float from, float to) const;
    std::vector<CompositionDependency> fullOrderedDependency(float from, float to) const;

private:
    std::string getFramePath(int f) const;
    // data
    std::string name = "";

    std::vector<Layer> layers;

    std::map<int, bool> rendered;
};

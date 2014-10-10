#pragma once
#include "global.hpp"
#include "renderable.hpp"

class CCPlus::Composition: public CCPlus::Renderable {
public:
    Composition(const std::string& name, float duration, int width, int height);
    void appendLayer(const Layer&);

    std::vector<Layer> layers;
    std::string name;
    float duration;
    float width;
    float height;
};

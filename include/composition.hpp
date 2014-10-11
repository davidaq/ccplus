#pragma once
#include "global.hpp"
#include "renderable.hpp"

class CCPlus::Composition: public CCPlus::Renderable {
public:
    Composition(const std::string& name, float duration, int width, int height);
    void appendLayer(const Layer&);

    float getDuration();
    void updateGPUFrame(GPUFrame& frame, float time);

    std::vector<Layer> layers;
    std::vector<GPUFrame> frames;
    std::string name;
    float duration;
    float width;
    float height;
};

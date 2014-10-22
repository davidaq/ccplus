#pragma once
#include "global.hpp"
#include "renderable.hpp"

class CCPlus::Composition: public CCPlus::Renderable {
public:
    Composition(float duration, int width, int height);

    void appendLayer(const Layer&);
    float getDuration();
    GPUFrame getGPUFrame(float time);
    void release();

    std::vector<Layer> layers;
    float duration;
    float width;
    float height;

    float lastQuery[2] = {-1,-1};
    GPUFrame lastFrame[2];
};

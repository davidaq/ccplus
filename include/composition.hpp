#pragma once
#include "global.hpp"
#include "renderable.hpp"

class CCPlus::Composition: public CCPlus::Renderable {
public:
    Composition(float duration, int width, int height);

    void prepare();
    void release();
    void appendLayer(const Layer&);
    float getDuration();
    GPUFrame getGPUFrame(float time);

    std::vector<Layer> layers;
    float duration;
    float width;
    float height;

    float lastQuery = -1;
    GPUFrame lastFrame;
    bool stillCached = false;
    bool still = false;
    bool isStill();
};

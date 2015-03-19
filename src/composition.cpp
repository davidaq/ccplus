#include "composition.hpp"
#include "layer.hpp"
#include "render.hpp"
#include "gpu-frame.hpp"
#include "context.hpp"

using namespace CCPlus;

Composition::Composition(float duration, float width, float height) {
    this->duration = duration;
    this->width = width;
    this->height = height;
    potWidth = nearestPOT(width);
    potHeight = nearestPOT(height);
    scaleAdjustX = width / potWidth;
    scaleAdjustY = height / potHeight;
}

void Composition::appendLayer(const Layer& layer) {
    layers.push_back(layer);
}

bool Composition::isStill() {
    if(stillCached)
        return still;
    for (auto& l : layers) {
        if (!l.still()) {
            stillCached = true;
            still = false;
            return false;
        }
    }
    stillCached = true;
    still = true;
    return true;
}

void Composition::prepare() {
    isStill();
}

void Composition::release() {
    lastFrame = GPUFrame();
}

GPUFrame Composition::getGPUFrame(float time) {
    if(lastFrame) {
        if (still || std::abs(time - lastQuery) < 0.0001)
            return lastFrame;
    }
    // Apply filters & track matte
    GPUFrame* frames = new GPUFrame[layers.size()];
    for (int i = 0; i < layers.size(); i++) {
        Layer& l = layers[i];
        if(!l.visible(time))
            continue;
        if(i < layers.size() - 1) {
            if(!l.show && layers[i + 1].trkMat == 0)
                continue;
        } else if(!l.show)
            continue;
        frames[i] = l.getFilteredFrame(time);
        if(i > 0 && l.trkMat) {
            frames[i] = trackMatte(frames[i], frames[i - 1], (TrackMatteMode)l.trkMat);
        }
    }
    // Merge & track matte 
    GPUFrame ret = GPUFrameCache::alloc(potWidth, potHeight);
    // make sure clean
    if(!transparent)
        glClearColor(0, 0, 0, 1.0f);
    ret->bindFBO();
    if(!transparent)
        glClearColor(0, 0, 0, 0);
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.show || !l.visible(time))
            continue;
        GPUFrame cframe = frames[i];
        ret = mergeFrame(ret, cframe, (BlendMode)l.blendMode);
    }
    delete[] frames;
    ret->ext.scaleAdjustX = scaleAdjustX;
    ret->ext.scaleAdjustY = scaleAdjustY;
    lastQuery = time;
    lastFrame = ret;
    return ret;
}

float Composition::getDuration() {
    return duration;
}

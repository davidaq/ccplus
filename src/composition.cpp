#include "composition.hpp"
#include "layer.hpp"
#include "render.hpp"
#include "gpu-frame.hpp"
#include "context.hpp"

using namespace CCPlus;

Composition::Composition(float duration, int width, int height) {
    this->duration = duration;
    this->width = width;
    this->height = height;
}

void Composition::appendLayer(const Layer& layer) {
    layers.push_back(layer);
}

void Composition::prepare() {
    for (auto& l : layers) {
        if (!l.still()) {
            still = false;
            return;
        }
    }
    L() << "still composition!";
    still = true;
}

GPUFrame Composition::getGPUFrame(float time) {
    if (std::abs(time - lastQuery) < 0.0001)
        return lastFrame;
    if (lastQuery != -1 && still) 
        return lastFrame;
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
    GPUFrame ret = GPUFrameCache::alloc(width, height);
    // make sure clean
    ret->bindFBO();
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.show || !l.visible(time))
            continue;
        GPUFrame cframe = frames[i];
        ret = mergeFrame(ret, cframe, (BlendMode)l.blendMode);
    }
    delete[] frames;
    lastQuery = time;
    lastFrame = ret;
    return ret;
}

float Composition::getDuration() {
    return duration;
}

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

GPUFrame Composition::getGPUFrame(float time) {
    // Apply filters
    GPUFrame* frames = new GPUFrame[layers.size()];
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!layers[i + 1].trkMat && (!l.show || !l.visible(time))) {
            frames[i] = GPUFrame();
        } else {
            frames[i] = l.getFilteredFrame(time);
        }
    }
    // Merge & track matte 
    GPUFrame ret;
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.show || !l.visible(time))
            continue;
        if(ret) {
            GPUFrame cframe = frames[i];
            if(i != 0 && l.trkMat) {
                cframe = trackMatte(frames[i], frames[i - 1], (TrackMatteMode)l.trkMat);
            }
            ret = mergeFrame(ret, cframe, (BlendMode)l.blendMode);
        } else {
            ret = frames[i];
        }
    }
    delete[] frames;
    return ret;
}

float Composition::getDuration() {
    return duration;
}

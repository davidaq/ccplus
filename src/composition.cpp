#include "composition.hpp"
#include "layer.hpp"

using namespace CCPlus;

Composition::Composition(const std::string& name, float duration, int width, int height) {
    this->name = name;
    this->duration = duration;
    this->width = width;
    this->height = height;
}

void Composition::appendLayer(const Layer& layer) {
    layers.push_back(layer);
    GPUFrame f;
    frames.push_back(f);
}

void Composition::updateGPUFrame(GPUFrame& frame, float time) {
    if (!frame.textureID) {
        frame.createTexture(width, height);
    }
    // Clean screen ~
    frame.bindFBO();
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        l.applyFiltersToFrame(frames[i], time);

        mergeFrame(frames[i], frame, 0);
    }
}

float Composition::getDuration() {
    return duration;
}

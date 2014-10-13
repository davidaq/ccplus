#include "composition.hpp"
#include "layer.hpp"
#include "render.hpp"
#include "gpu-frame.hpp"

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
    GPUFrame secondary;
    secondary.createTexture(width, height);
    GPUFrame* dblBuffer[2] = {&frame, &secondary};
    int currentSrc = 0;
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.visible(time))
            continue;
        l.applyFiltersToFrame(frames[i], time);

        int currentBuffer = currentSrc ^ 1;
        dblBuffer[currentBuffer]->bindFBO();
        glClear(GL_COLOR_BUFFER_BIT);
        mergeFrame(*dblBuffer[currentSrc], frames[i], DEFAULT);
        currentSrc = currentBuffer;
    }
    if(currentSrc == 1) {
        frame.destroy();
        frame.textureID = secondary.textureID;
        frame.fboID = secondary.fboID;
        frame.audio = secondary.audio;
    } else {
        secondary.destroy();
    }
}

float Composition::getDuration() {
    return duration;
}

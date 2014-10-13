#include "composition.hpp"
#include "layer.hpp"
#include "render.hpp"
#include "gpu-frame.hpp"
#include "gpu-double-buffer.hpp"

using namespace CCPlus;

Composition::Composition(const std::string& name, float duration, int width, int height) {
    this->name = name;
    this->duration = duration;
    this->width = width;
    this->height = height;
}

void Composition::appendLayer(const Layer& layer) {
    layers.push_back(layer);
    frames.push_back(GPUFrame());
    filteredFrames.push_back(GPUFrame());
}

void Composition::updateGPUFrame(GPUFrame& frame, float time) {
    if (!frame.textureID) {
        frame.createTexture(width, height);
    }
    frame.bindFBO();
    glClear(GL_COLOR_BUFFER_BIT);
    GPUDoubleBuffer buffer(frame, width, height);
    // Apply filters
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.visible(time))
            continue;
        l.applyFiltersToFrame(frames[i], buffer, time);
        GPUFrame& filteredFrame = filteredFrames[i];
        buffer.swap([&filteredFrame](GPUFrame& src) {
            filteredFrame.bindFBO();
            glClear(GL_COLOR_BUFFER_BIT);
            mergeFrame(src, src, NONE);
        });
    }
    // Clean screen ~
    GPUFrame matteBuffer;
    matteBuffer.createTexture(width, height);
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.show || !l.visible(time))
            continue;
        if(i != 0 && l.trkMat) {
            matteBuffer.bindFBO();
            glClear(GL_COLOR_BUFFER_BIT);
            trackMatte(frames[i], frames[i - 1], (TrackMatteMode)l.trkMat);

            buffer.swap([&matteBuffer](GPUFrame& source) {
                mergeFrame(source, matteBuffer, DEFAULT);
            });
        } else {
            GPUFrame &cframe = frames[i];
            buffer.swap([&cframe](GPUFrame& source) {
                glClear(GL_COLOR_BUFFER_BIT);
                mergeFrame(source, cframe, DEFAULT);
            });
        }
    }
    matteBuffer.destroy();
    buffer.finish();
}

float Composition::getDuration() {
    return duration;
}

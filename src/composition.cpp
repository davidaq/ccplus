#include "composition.hpp"
#include "layer.hpp"
#include "render.hpp"
#include "gpu-frame.hpp"
#include "gpu-double-buffer.hpp"
#include "context.hpp"

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
    frame.audio = cv::Mat();
    // Apply filters & merge audio
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.show || !l.visible(time)) {
            // frame will be destroyed once not visible
            frames[i].destroy();
            filteredFrames[i].destroy();
            // TODO check and release renderable
            continue;
        }
        l.applyFiltersToFrame(frames[i], filteredFrames[i], time);
        mergeAudio(frame.audio, filteredFrames[i].audio);
    }
    // Merge & track matte 
    GPUDoubleBuffer buffer(frame, width, height);
    GPUFrame matteBuffer;
    matteBuffer.createTexture(width, height);
    for (int i = layers.size() - 1; i >= 0; i--) {
        Layer& l = layers[i];
        if(!l.visible(time))
            continue;
        if(i != 0 && l.trkMat) {
            matteBuffer.bindFBO();
            glClear(GL_COLOR_BUFFER_BIT);
            trackMatte(filteredFrames[i], filteredFrames[i - 1], (TrackMatteMode)l.trkMat);
            buffer.swap([&matteBuffer, &l](GPUFrame& source) {
                glClear(GL_COLOR_BUFFER_BIT);
                return mergeFrame(source, matteBuffer, (BlendMode)l.blendMode);
            });
        } else {
            GPUFrame &cframe = filteredFrames[i];
            buffer.swap([&cframe, &l](GPUFrame& source) {
                glClear(GL_COLOR_BUFFER_BIT);
                return mergeFrame(source, cframe, (BlendMode)l.blendMode);
            });
        }
    }
    matteBuffer.destroy();
    buffer.finish();
}

float Composition::getDuration() {
    return duration;
}

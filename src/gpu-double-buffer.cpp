#include "gpu-double-buffer.hpp"
#include "gpu-frame.hpp"

using namespace CCPlus;

GPUDoubleBuffer::GPUDoubleBuffer(int width, int height) {
    source = new GPUFrame();
    source->createTexture(width, height);
    secondary = new GPUFrame();
    secondary->createTexture(width, height);
    cSource = cSecondary = true;
    dblBuffer[0] = this->source;
    dblBuffer[1] = this->secondary;
}

GPUDoubleBuffer::GPUDoubleBuffer(GPUFrame& source, int width, int height) {
    this->source = &source;
    secondary = new GPUFrame();
    secondary->createTexture(width, height);
    cSource = false;
    cSecondary = true;
    dblBuffer[0] = this->source;
    dblBuffer[1] = this->secondary;
}

GPUDoubleBuffer::GPUDoubleBuffer(GPUFrame& source, GPUFrame& secondary) {
    this->source = &source;
    this->secondary = &secondary;
    cSource = false;
    cSecondary = false;
    dblBuffer[0] = this->source;
    dblBuffer[1] = this->secondary;
}

GPUDoubleBuffer::~GPUDoubleBuffer() {
    finish();
    if(cSource) {
        source->destroy();
        delete source;
    }
    if(cSecondary) {
        secondary->destroy();
        delete secondary;
    }
}

void GPUDoubleBuffer::finish() {
    if(finished)
        return;
    finished = true;
    if(currentSrc == 1) {
        source->destroy();
        source->textureID = secondary->textureID;
        source->fboID = secondary->fboID;
        source->audio = secondary->audio;
    } else {
        secondary->destroy();
    }
}

void GPUDoubleBuffer::swap(const std::function<void(CCPlus::GPUFrame&)>& func) {
    int currentBuffer = currentSrc ^ 1;
    dblBuffer[currentBuffer]->bindFBO();
    func(*dblBuffer[currentSrc]);
    currentSrc = currentBuffer;
}


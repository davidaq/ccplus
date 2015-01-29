#pragma once
#include "renderable.hpp"

// @ static image, period.
class CCPlus::ImageRenderable : public CCPlus::Renderable {
public:
    ImageRenderable(const std::string& uri);
    ~ImageRenderable() {this->release();}

    GPUFrame getGPUFrame(float time);
    void prepare();
    void prepareWithFileData(const uint8_t* data, uint32_t len, int rotate=0);
    void release();
    void releaseGPUCache();
    float getDuration();
    
    int getWidth() const;
    int getHeight() const;

protected:
    Frame image;
    GPUFrame gpuCache;
    std::string uri;
};


#pragma once
#include "renderable.hpp"

class CCPlus::ColorRenderable : public CCPlus::Renderable {
public:
    ColorRenderable(const std::string& uri);
    ~ColorRenderable() {this->release();};

    GPUFrame getGPUFrame(float time);
    void prepare();
    void release();
    float getDuration();
    
    int getWidth() const;
    int getHeight() const;

protected: 
    Frame image;
    GPUFrame cache;
    std::string uri;
    int width = 0;
    int height = 0;
    float r = 0, g = 0, b = 0;
};

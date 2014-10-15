#pragma once
#include "renderable.hpp"

// @ static image, period.
class CCPlus::ImageRenderable : public CCPlus::Renderable {
public:
    ImageRenderable(const std::string& uri);

    void getGPUFrame(float time);
    void prepare();
    void release();
    float getDuration();
    
    int getWidth() const;
    int getHeight() const;

protected:
    Frame image;
    std::string uri;
};


#pragma once
#include "renderable.hpp"

// @ static image, period.
class CCPlus::ImageRenderable : public CCPlus::Renderable {
public:
    ImageRenderable(const std::string& uri);

    Frame getFrame(float);
    void prepare();
    void release();
    float getDuration();
    
    float getDuration() const;
    int getWidth() const;
    int getHeight() const;

protected:
    Frame image;
    std::string uri;
};


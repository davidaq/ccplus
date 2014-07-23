#pragma once

#include "animated-renderable.hpp"

namespace CCPlus {
    class VideoRenderable;
    class VideoDecoder;
}

// @ handle videos, make'm to preprocessed image
class CCPlus::VideoRenderable : public CCPlus::AnimatedRenderable {
public:
    VideoRenderable(CCPlus::Context* context, const std::string& uri);
    ~VideoRenderable();

    float getDuration() const;
    int getWidth() const;
    int getHeight() const;
private:
    
    VideoDecoder* decoder;

    std::string getFramePath(int f) const;
    void renderPart(float start, float duration);
};


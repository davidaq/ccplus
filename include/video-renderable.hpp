#pragma once

#include "animated-renderable.hpp"

namespace CCPlus {
    class VideoRenderable;
    class VideoDecoder;
}

// @ handle videos, make'm to preprocessed image
class CCPlus::VideoRenderable : public CCPlus::AnimatedRenderable {
public:
    VideoRenderable(const std::string& uri);
    ~VideoRenderable();

    float getDuration() const;
    int getWidth() const;
    int getHeight() const;
    const std::string& getName() const;

    bool still(float t1, float t2);

private:

    std::string uri;
    
    VideoDecoder *decoder, *alpha_decoder;

    void renderPart(float start, float duration);
};


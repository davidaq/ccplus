#pragma once

#include "animated-renderable.hpp"
#include "context.hpp"

namespace CCPlus {
    class AudioRenderable;
    class VideoDecoder;
}

class CCPlus::AudioRenderable : public CCPlus::AnimatedRenderable {
public:
    AudioRenderable(CCPlus::Context* ctx, const std::string& uri);
    ~AudioRenderable();

    std::string getPCMPath() const;
    Image getFrame(float time) const;
    
    int getWidth() const;
    int getHeight() const;

protected:
    void renderPart(float, float);
    VideoDecoder* decoder;
};


#pragma once

#include "animated-renderable.hpp"

#define GIF_FPS 15

namespace CCPlus {
    class GifRenderable;
}

struct GifFileType;

// @ handle gif animations
class CCPlus::GifRenderable : public CCPlus::AnimatedRenderable {
public:
    GifRenderable(CCPlus::Context* context, const std::string& uri);
    ~GifRenderable();

    float getDuration() const;
    int getWidth() const;
    int getHeight() const;
    const std::string& getName() const;

    bool still(float t1, float t2);
private:

    std::string uri;

    GifFileType* ctx = 0;

    void renderPart(float start, float duration);
};

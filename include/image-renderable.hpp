#pragma once

#include "renderable.hpp"

namespace CCPlus {
    class ImageRenderable;
}

// @ static image, period.
class CCPlus::ImageRenderable : public CCPlus::Renderable {
public:
    ImageRenderable(CCPlus::Context* context, const std::string& uri);

    void render(float start, float duration);
    Image getFrame(float time) const;
    
    float getDuration() const;
    int getWidth() const;
    int getHeight() const;

protected:
    Image image;
};


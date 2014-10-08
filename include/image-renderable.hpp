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
    Frame getFrame(float time) const;
    Frame getFrameByNumber(int frame) const;
    
    float getDuration() const;
    int getWidth() const;
    int getHeight() const;

    const std::string& getName() const;

    bool still(float t1, float t2);

    void clear();

protected:
    std::string getFramePath(int f) const;
    Frame cache;
    bool loaded = false;
    std::string path;

    std::string uri;
};


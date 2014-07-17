#pragma once
#include "global.hpp"

// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable(CCPlus::Context*, float, int, int);
    
    virtual void render(float start, float duration) = 0;
    virtual Image getFrame(float time) const = 0;
    
    float getDuration() const;

    int getWidth() const;

    int getHeight() const;

    int getFrameNumber(float time) const;

protected:

    std::string uuid;

    CCPlus::Context* context = 0;
    
    float duration = 0;
    int width = 0;
    int height = 0;
};

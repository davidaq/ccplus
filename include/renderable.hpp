#pragma once
#include "object.hpp"
#include "image.hpp"

namespace CCPlus {
    class Renderable;
    class Context;
}
// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable(CCPlus::Context*);
    
    virtual void render(float start, float duration) = 0;
    virtual Image getFrame(float time) const = 0;
    
    virtual float getDuration() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    int getFrameNumber(float time) const;

protected:

    std::string uuid;

    CCPlus::Context* context = 0;
    
    float duration = 0;
    int width = 0;
    int height = 0;
};

#pragma once
#include "object.hpp"
#include "frame.hpp"

namespace CCPlus {
    class Renderable;
    class Context;
}
// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable(CCPlus::Context*);
    
    virtual void render(float start, float duration) = 0;
    virtual Frame getFrame(float time) const = 0;
    virtual Frame getFrameByNumber(int frame) const = 0;

    virtual const std::string& getName() const = 0;
    
    virtual float getDuration() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    virtual void clear() = 0;

    /**
     * Chech if this renderable *LOOKS* the same at
     * time @t1 and @t2
     * Note: they might have different audio
     */
    virtual bool still(float t1, float t2) = 0;

    int getFrameNumber(float time) const;

    float getFrameTime(int frame) const;


protected:
    std::string getFramePath(int f) const;

    std::string uuid;

    CCPlus::Context* context = 0;
};

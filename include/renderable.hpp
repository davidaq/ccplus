#pragma once
#include "global.hpp"

// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable(float, float, float);
    
    // virtual void render(float start,float duration) = 0;
    // virtual Image getFrame(float time) = 0;
    
    float getDuration() const;

    float getWidth() const;

    float getHeight() const;

protected:
    
    float duration = 0;
    float width = 0;
    float height = 0;
};
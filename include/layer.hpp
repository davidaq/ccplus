#pragma once
#include "global.hpp"

class CCPlus::Layer : public CCPlus::Object {

public:
    Layer();
    Layer(
        CCPlus::Renderable* renderobject, 
        float time, 
        float duration, 
        float start, 
        float last
    );

    // access
    CCPlus::Renderable* getRenderObject() const;
    float getTime() const;
    float getDuration() const;
    float getStart() const;
    float getLast() const;
private:
    // data
    CCPlus::Renderable* renderObject = 0;
    
    float time = 0, duration = 0, start = 0, last = 0;
};

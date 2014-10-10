#pragma once
#include "global.hpp"
#include "frame.hpp"

// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable();
    ~Renderable();

    virtual Frame getFrame(int time) = 0;
    virtual void prepare() = 0;
    virtual int getDuration() = 0;
private:
    std::map<int, Frame> frames;
    float firstAppearTime = 0, lastAppearTime = 0;
    std::vector<std::pair<float,float> > usedFragments;
};

#pragma once
#include "global.hpp"
#include "frame.hpp"

// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable();
    ~Renderable();

    virtual void prepare();
    virtual void release();
    virtual float getDuration();

    GPUFrame getWrapedGPUFrame(float time);
    virtual GPUFrame getGPUFrame(float time);

    static std::string parseUri2File(std::string uri);

    // first and last moments this renderable is visible in the main composition
    float firstAppearTime = 0, lastAppearTime = 0;
    // chunks of used fragments of this renderable as <start,end>
    std::vector<std::pair<float,float> > usedFragments;
    std::string getUri();
private:
    std::map<int, Frame> frames;
    std::string uri;
};

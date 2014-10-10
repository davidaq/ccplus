#pragma once
#include "global.hpp"
#include "frame.hpp"

// @ base class of anything that could be referenced by a layer
class CCPlus::Renderable : public CCPlus::Object {
public:
    Renderable();
    ~Renderable();

    virtual Frame getFrame(float time);
    virtual void prepare();
    virtual void release();
    virtual float getDuration();

    static inline std::string parseUri2File(const std::string& uri) {
        return uri;
    }
private:
    std::map<int, Frame> frames;
    float firstAppearTime = 0, lastAppearTime = 0;
    std::vector<std::pair<float,float> > usedFragments;
};

#pragma once
#include "renderable.hpp"

namespace CCPlus {
    class AnimatedRenderable;
}

// @ base class of any renderable that would contain animations

class CCPlus::AnimatedRenderable : public CCPlus::Renderable {
public:
    AnimatedRenderable(CCPlus::Context*);

    void render(float start, float duration);
    Frame getFrame(float time) const;
    Frame getFrameByNumber(int frame) const;

protected:
    virtual void renderPart(float start, float duration) = 0;
    std::set<int> rendered;
    std::string getFramePath(int f) const;
};


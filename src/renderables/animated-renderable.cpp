#include "animated-renderable.hpp"
#include "context.hpp"

using namespace CCPlus;

AnimatedRenderable::AnimatedRenderable() :
    Renderable()
{
}

void AnimatedRenderable::render(float start, float duration) {
    log(logINFO) << "\t Rendering " << this->getName() << " from " << 
        start << " to " << start + duration;
    if(duration + 0.001 > getDuration()) {
        renderPart(0, getDuration());
    } else {
        while(start > getDuration())
            start -= getDuration();
        float end = start + duration;
        while(end > getDuration())
            end -= getDuration();
        if(start < end) {
            renderPart(start, end - start);
        } else {
            renderPart(0, end);
            renderPart(start, getDuration() - start);
        }
    }
    log(logINFO) << "\t Done";;
}

Frame AnimatedRenderable::getFrame(float time) const {
    while(time > getDuration())
        time -= getDuration();
    int f = getFrameNumber(time);
    return getFrameByNumber(f);
}

Frame AnimatedRenderable::getFrameByNumber(int frame) const {
    if(rendered.count(frame)) {
        return Frame(getFramePath(frame));
    }
    return Frame();
}

void AnimatedRenderable::clear() {
    rendered.clear();
}

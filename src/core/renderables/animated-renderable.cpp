#include "animated-renderable.hpp"
#include "context.hpp"

using namespace CCPlus;

AnimatedRenderable::AnimatedRenderable(Context* context) :
    Renderable(context)
{
}

std::string AnimatedRenderable::getFramePath(int f) const {
    return context->getStoragePath() + "/" + uuid + "_" + std::to_string(f) + ".zim";
}

void AnimatedRenderable::render(float start, float duration) {
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
}

Image AnimatedRenderable::getFrame(float time) const {
    while(time > getDuration())
        time -= getDuration();
    int f = getFrameNumber(time);
    if(rendered.count(f)) {
        return Image(getFramePath(f));
    }
    return Image();
}

#include "text-renderable.hpp"
#include "frame.hpp"
#include "logger.hpp"

using namespace cv;
using namespace CCPlus;

TextRenderable::TextRenderable(Context* context, 
        const std::string& _uri) 
    : Renderable(context), uri(_uri) {
    
} 

const std::string& TextRenderable::getName() const {
    return uri;
}

void TextRenderable::clear() {
}

int TextRenderable::getWidth() const {
    return 0;
}

int TextRenderable::getHeight() const {
    return 0;
}

float TextRenderable::getDuration() const {
    return 0x7fffffff;
}

bool TextRenderable::still(float t1, float t2) {
    return findKeyByTime(t1) == findKeyByTime(t2);
}

Frame TextRenderable::getFrame(float time) const {
    return getFrameByNumber(getFrameNumber(time));
}

Frame TextRenderable::getFrameByNumber(int frame) const {
    if (keyframes.empty()) {
        log(logWARN) << "Access to a unrendered text object";
        return Frame();
    }
    int retf = findKey(frame);
    if (rendered.count(retf))
        return Frame(getFramePath(retf));
    return Frame();
}

void TextRenderable::render(float start, float duration) {
    // Generate key frame list
    if (keyframes.empty()) {
        auto f = [&] (float t) {
            keyframes.push_back(getFrameNumber(t));
        };
        for (auto& kv : text)  f(kv.first);
        for (auto& kv : font)  f(kv.first);
        for (auto& kv : size)  f(kv.first);
        for (auto& kv : scale_x)  f(kv.first);
        for (auto& kv : scale_y)  f(kv.first);
        for (auto& kv : tracking)  f(kv.first);
        for (auto& kv : bold)  f(kv.first);
        for (auto& kv : italic)  f(kv.first);
        sort(keyframes.begin(), keyframes.end());
        unique(keyframes.begin(), keyframes.end());
        if (keyframes.empty()) {
            log(logWARN) << "TextRenderable hasn't been given parameters";
        }
    }

    int startFrame = getFrameNumber(start);
    int endFrame = getFrameNumber(start + duration);
    for (int i = startFrame; i <= endFrame; i++) {
        // Come on FreeType!
    }
}

int TextRenderable::findKey(int f) const {
    if (keyframes.empty())
        return 0;
    for (int i = keyframes.size() - 1; i >= 0; i--) {
        if (keyframes[i] <= f) 
            return keyframes[i];
    }
    log(logWARN) << "TextRenderable: Couldn't find key frame for " << f;
    return keyframes[0];
}

int TextRenderable::findKeyByTime(float t) const {
    return findKey(getFrameNumber(t));
}

#include "renderable.hpp"
#include "context.hpp"

using namespace CCPlus;

Renderable::Renderable() {
    refCounter = 0;
}

Renderable::~Renderable() {
}

void Renderable::prepare() {
}

void Renderable::release() {
}

float Renderable::getDuration() {
    return 0;
}

std::string Renderable::parseUri2File(std::string uri) {
    if(stringStartsWith(uri, "file://")) {
        uri = uri.substr(7);
    } else if(stringStartsWith(uri, "xfile://")) {
        uri = uri.substr(8);
    }
    return Context::getContext()->getFootagePath(uri);
}

GPUFrame Renderable::getGPUFrame(float time) {
    return GPUFrame();
}

GPUFrame Renderable::getWrapedGPUFrame(float time) {
    if(usedFragmentSlices.empty())
        return GPUFrame();
    float d = getDuration();
    float duration = d + 1.0 / frameRate;
    while(time < 0)
        time += duration;
    while(time >= duration)
        time -= duration;
    if(time > d)
        time = d;
    return getGPUFrame(time);
}

std::string Renderable::getUri() {
    if(uri.empty()) {
        for(const auto & item : Context::getContext()->renderables) {
            if(item.second == this) {
                uri = item.first;
                return uri;
            }
        }
    }
    return uri;
}

void Renderable::preparePart(float from, float duration) {
    refCounter++;
    prepare();
}

void Renderable::releasePart(float from, float duration) {
    refCounter--;
    if(refCounter <= 0) {
        refCounter = 0;
        release();
    }
}

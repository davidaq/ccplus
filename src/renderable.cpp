#include "renderable.hpp"
#include "context.hpp"

using namespace CCPlus;

Renderable::Renderable() {}

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
    }
    return Context::getContext()->getFootagePath(uri);
}

GPUFrame Renderable::getGPUFrame(float time) {
    return GPUFrame();
}

GPUFrame Renderable::getWrapedGPUFrame(float time) {
    if(usedFragments.empty())
        return GPUFrame();
    float duration = getDuration();
    while(time <= 0)
        time += duration;
    while(time > duration)
        time -= duration;
    return getGPUFrame(time);
}

std::string Renderable::getUri() {
    if(uri.empty()) {
        for(const auto & item : Context::getContext()->renderables) {
            if(item.second == this) {
                uri = item.first;
                break;
            }
        }
    }
    return uri;
}

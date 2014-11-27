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
        for(const auto & item : Context::getContext()->preservedRenderable) {
            if(item.second == this) {
                uri = item.first;
                return uri;
            }
        }
    }
    return uri;
}

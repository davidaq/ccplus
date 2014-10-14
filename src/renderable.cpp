#include "renderable.hpp"
#include "context.hpp"

using namespace CCPlus;

Renderable::Renderable() {}

Renderable::~Renderable() {}

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

void Renderable::updateGPUFrame(GPUFrame& frame, float time) {
}

void Renderable::updateWrapedGPUFrame(GPUFrame& frame, float time) {
    float duration = getDuration();
    while(time <= 0)
        time += duration;
    while(time > duration)
        time -= duration;
    updateGPUFrame(frame, time);
}


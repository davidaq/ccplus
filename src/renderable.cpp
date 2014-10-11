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

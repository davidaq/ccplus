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

std::string Renderable::parseUri2File(std::string uri, int* rotate) {
    if(stringStartsWith(uri, "file://")) {
        uri = uri.substr(7);
    } else if(stringStartsWith(uri, "xfile://")) {
        uri = uri.substr(8);
    }
    if(uri[0] == '?') {
        for(int i = 1; i < uri.size(); i++) {
            if(uri[i] == '?') {
                if(rotate)
                    *rotate = atoi(uri.substr(1, i - 1).c_str());
                uri = uri.substr(i + 1);
                break;
            }
        }
    }
#ifdef __IOS__
    if(stringStartsWith(uri, "assets-library://")) {
        return uri;
    }
#endif
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
    if(refCounter <= 0) {
        refCounter = 0;
        prepare();
    }
    refCounter++;
}

void Renderable::releasePart(float from, float duration) {
    refCounter--;
    if(refCounter <= 0) {
        refCounter = 0;
        release();
    }
}

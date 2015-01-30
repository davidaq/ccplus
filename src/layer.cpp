#include "layer.hpp"

#include "context.hpp"
#include "gpu-frame.hpp"
#include "renderable.hpp"
#include "composition.hpp"
#include "render.hpp"
#include "ccplus.hpp"
#include "filter.hpp"

#include "image-renderable.hpp"

using namespace CCPlus;

Layer::Layer() {}


Layer::Layer(
    const std::string& _renderableUri, 
    float _time, 
    float _duration, 
    float _start, 
    float _last,
    float _width,
    float _height,
    int _blendMode,
    int _trkMat,
    bool _showup,
    bool _motionBlur
) :
    time(_time),
    duration(_duration),
    start(_start),
    last(_last),
    blendMode(_blendMode),
    trkMat(_trkMat),
    show(_showup),
    motionBlur(_motionBlur),
    renderableUri(_renderableUri),
    width(_width),
    height(_height)
{
}

Renderable* Layer::getRenderObject() {
    if(!renderObject)
        renderObject = Context::getContext()->getRenderable(renderableUri);
    return renderObject;
}

bool Layer::visible(float t) const {
    if (t < this->time || t > this->duration + this->time) 
        return false;
    return true;
}

void Layer::setProperties(const std::map<std::string, Property>& prop) {
    this->properties = prop;
}

const std::map<std::string, Property>& Layer::getProperties() const {
    return properties;
}

std::vector<float> Layer::interpolate(const std::string& name, float time) const {
    std::vector<float> ret;
    if (properties.find(name) == properties.end()) {
        return ret;
    }
    const Property& prop = properties.at(name);
    /*
     * If prop only has one set of parameters at time 0
     * Return it
     */
    if (prop.size() == 1 && prop.count(0) != 0)
        return prop.at(0);
    float low_time, high_time;
    const std::vector<float>* low = nullptr;
    const std::vector<float>* high = nullptr;
    if (prop.find(time) != prop.end())
        return prop.at(time);

    for (const auto& kv : prop ) {
        if (kv.first < time) {
            low_time = kv.first;
            low = &kv.second;
        }
        if (kv.first > time) {
            high_time = kv.first;
            high = &kv.second;
            break; // Because map are always sorted 
        }
    }
    if (low == nullptr || high == nullptr) {
        log(logWARN) << "Parameter for " << name << " are not interpolatable at time " << time;
        return ret;
    }

    for (int i = 0; i < low->size(); i++) {
        float y = high->at(i);
        float x = low->at(i);
        float tmp = (time - low_time) / (high_time - low_time);
        tmp = tmp * (y - x) + x; 

        ret.push_back(tmp);
    }
    
    return ret;
}

#define MAX_BLUR_DIFF 40
static inline int diff(const std::vector<float>& a1, const std::vector<float>& a2) {
    if(a1.size() == a2.size()) {
        float diff = 0;
        for(int i = 0, c = a1.size(); i < c; i++) {
            const float& v = abs(a1[i] - a2[i]);
            diff += v;
            if(diff > MAX_BLUR_DIFF)
                break;
        }
        return diff;
    } else {
        return 0xfffffff;
    }
}

GPUFrame Layer::getFilteredFrame(float t) {
    if (!visible(t) || !getRenderObject())
        return GPUFrame();
    float local_t = mapInnerTime(t);
    GPUFrame frame = getRenderObject()->getWrapedGPUFrame(local_t);
    if(frame) {
        for (auto& k : (*filterOrder)) {
            if (!properties.count(k)) continue;
            std::vector<float> params;
            if(k == "transform") {
                params.reserve(25);
                params = interpolate("opacity", t);
                if(params.empty())
                    params.push_back(1.0);
                if(renderMode == FINAL_MODE && motionBlur) {
                    float blurTime = 1.0 / frameRate;
                    const std::vector<float>& right = interpolate(k, t);
                    std::vector<float> left = interpolate(k, t - blurTime);
                    params.insert(params.end(), right.begin(), right.end());
                    int d = diff(left, right);
                    const static float minStep = 0.0002;
                    while(d > MAX_BLUR_DIFF && blurTime > minStep) {
                        blurTime /= 2;
                        left = interpolate(k, t - blurTime);
                        d = diff(left, right);
                    }
                    if(d > 0 && blurTime > minStep) {
                        float step = blurTime / 20;
                        if(step < minStep)
                            step = minStep;
                        for(float b = step; b < blurTime; b += step) {
                            left = interpolate(k, t - b);
                            static const float sep[] = {0,0,0,0,0,0,0,0,0,0,0,0};
                            params.reserve(params.size() + left.size() + 12);
                            params.insert(params.end(), sep, sep + 12);
                            params.insert(params.end(), left.begin(), left.end());
                        }
                    }
                } else {
                    std::vector<float> p = interpolate(k, t);
                    params.reserve(1 + p.size());
                    params.insert(params.end(), p.begin(), p.end());
                }
            } else {
                params = interpolate(k, t);
            }
            frame = Filter(k).apply(frame, params, width, height);
        }
    }
    return frame;
}

float Layer::mapInnerTime(float t) const {
    return start + last / duration * (t - time);   
}

bool Layer::still() {
    Composition* comp = dynamic_cast<Composition*>(getRenderObject());
    if(comp && !comp->isStill())
        return false;
    if (!comp && !dynamic_cast<ImageRenderable*>(this->getRenderObject())) {
        return false;
    }
    for (auto& kv : properties) {
        auto& p = kv.second;
        if (p.size() <= 1) continue; 
        bool first = true;
        std::vector<float> ref;
        for (auto& kv2 : p) {
            auto& v = kv2.second;
            if (first) {
                first = false;
                ref = v;
            } else {
                if (v.size() != ref.size()) return false;
                for (int i = 0; i < v.size(); i++) {
                    if (v[i] != ref[i])
                        return false;
                }
            }
        }
    } 
    return true;
}

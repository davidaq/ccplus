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
    bool _motionBlur,
    bool _rawTransform
) :
    time(_time),
    duration(_duration),
    start(_start),
    last(_last),
    blendMode(_blendMode),
    trkMat(_trkMat),
    show(_showup),
    motionBlur(_motionBlur),
    rawTransform(_rawTransform),
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

#define MAX_BLUR_DIFF 500
static inline int diff(const std::vector<float>& a1, const std::vector<float>& a2) {
    if(a1.size() == a2.size()) {
        float diff = 0;
        for(int i = 0, c = a1.size(); i < c; i++) {
            const float& v = Fabs(a1[i] - a2[i]);
            const int& j = i % 12;
            if(j > 8)
                diff += v * 2;
            else if(j >5)
                diff += v;
            else
                diff += (int)v;
            if(diff > MAX_BLUR_DIFF)
                break;
        }
        return diff * 10;
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
                params = interpolate("opacity", t);
                params.reserve(26);
                if(params.empty())
                    params.push_back(1.0);
                params.push_back(rawTransform ? 1 : -1);
                if(motionBlur) {
                    float blurTime = renderMode == FINAL_MODE ? 0.05 : 0.02;
                    const std::vector<float>& right = interpolate(k, t);
                    std::vector<float> left = interpolate(k, t - blurTime);
                    params.insert(params.end(), right.begin(), right.end());
                    int d = diff(left, right);
                    const float& minStep = 0.0005;
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
                            const std::vector<float>& np = interpolate(k, t - b);
                            if(diff(left, np) > 5) {
                                left = np;
                                static const float sep[] = {0,0,0,0,0,0,0,0,0,0,0,0};
                                params.reserve(params.size() + left.size() + 12);
                                params.insert(params.end(), sep, sep + 12);
                                params.insert(params.end(), left.begin(), left.end());
                            }
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

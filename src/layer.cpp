#include "layer.hpp"

#include "context.hpp"
#include "frame.hpp"
#include "renderable.hpp"
#include "filter.hpp"

using namespace CCPlus;

Layer::Layer() {}


Layer::Layer(
    const std::string& _renderableUri, 
    float _time, 
    float _duration, 
    float _start, 
    float _last,
    int _width,
    int _height,
    int _blendMode,
    int _trkMat,
    bool _showup
) :
    renderableUri(_renderableUri),
    time(_time),
    duration(_duration),
    start(_start),
    last(_last),
    width(_width),
    height(_height), 
    blendMode(_blendMode),
    trkMat(_trkMat),
    show(_showup)
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

void Layer::setProperties(const std::map<std::string, Property>& prop, 
        const std::vector<std::string>& keyOrder) {
    this->orderedKey = keyOrder;
    properties = prop;
}

void Layer::setProperties(const std::map<std::string, Property>& prop) {
    properties = prop;
}

const std::map<std::string, Property>& Layer::getProperties() const {
    return properties;
}

// TODO: binary search needed here
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
        log(logWARN) << "Parametere for " << name << " are not interpolatable at time " << time;
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

Frame Layer::applyFiltersToFrame(float t) {
    if (!visible(t)) 
        return Frame();

    // Calculate corresponding local time
    float local_t = mapInnerTime(t);
    //Frame frame = this->getRenderObject()->getFrame(local_t);
    //if (orderedKey.empty()) {
    //    for (auto& kv : properties) {
    //        Filter(kv.first).apply(
    //                frame, interpolate(kv.first, t), width, height);
    //    }
    //} else {
    //    for (auto& k : orderedKey) {
    //        Filter(k).apply(frame, interpolate(k, t), width, height);
    //    }
    //}
    //return frame;
    return Frame();
}

float Layer::mapInnerTime(float t) const {
    return start + last / duration * (t - time);   
}

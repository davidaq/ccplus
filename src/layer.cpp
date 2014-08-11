#include "layer.hpp"
#include "filter.hpp"

using namespace CCPlus;

Layer::Layer() {}


Layer::Layer(
    Context* ctx,
    const std::string& _renderableUri, 
    float _time, 
    float _duration, 
    float _start, 
    float _last,
    int _width,
    int _height
) :
    context(ctx),
    renderableUri(_renderableUri),
    renderObject(0),
    time(_time),
    duration(_duration),
    start(_start),
    last(_last),
    width(_width),
    height(_height)
{
}

Renderable* Layer::getRenderObject() {
    if(!renderObject)
        renderObject = context->getRenderable(renderableUri);
    return renderObject;
}

float Layer::getTime() const {
    return time;
}

float Layer::getDuration() const {
    return duration;
}

float Layer::getStart() const {
    return start;
}

float Layer::getLast() const {
    return last;
}

void Layer::setProperties(const std::map<std::string, Property>& prop, 
        const std::vector<std::string>& keyOrder) {
    this->orderedKey = keyOrder;
    properties = prop;
}

void Layer::setProperties(const std::map<std::string, Property>& prop) {
    properties = prop;
}

std::map<std::string, Property> Layer::getProperties() const {
    return properties;
}

std::vector<float> Layer::interpolate(const std::string& name, float time) const {
    std::vector<float> ret;
    if (properties.find(name) == properties.end()) {
        return ret;
    }
    const Property& prop = properties.at(name);
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
        log(logWARN) << "Parametere are not interpolatable at time " << time;
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
    if (t < this->getTime() || t > this->getDuration() + this->getTime()) 
        return Frame();

    // Calculate corresponding local time
    float local_t = start + last / duration * (t - time);
    Frame frame = this->getRenderObject()->getFrame(local_t);
    if (orderedKey.empty()) {
        for (auto& kv : properties) 
        {
            const std::vector<float>& tmp = interpolate(kv.first, t);
            Filter(kv.first).apply(frame, 
                    tmp, width, height);
        }
    } else {
        for (auto& k : orderedKey)
        {
            const std::vector<float>& tmp = interpolate(k, t);
            Filter(k).apply(frame, tmp, width, height);
        }
    }
    return frame;
}

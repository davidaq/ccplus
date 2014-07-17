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

void Layer::setProperties(const std::map<std::string, Property>& prop) {
    properties = prop;
}

std::map<std::string, Property> Layer::getProperties() const {
    return properties;
}

std::vector<float> Layer::interpolate(const std::string& name, float time) const {
    std::vector<float> ret;
    if (properties.find(name) == properties.end()) {
        //std::cout << "Fatal error: this layer doesn't contains this property: " << name << std::endl;
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
    if (low == nullptr || high == nullptr) 
        return ret;

    for (int i = 0; i < low->size(); i++) {
        float y = high->at(i);
        float x = low->at(i);
        float tmp = (time - low_time) / (high_time - low_time);
        tmp = tmp * (y - x) + x; 

        ret.push_back(tmp);
    }
    
    return ret;
}

Image Layer::applyFiltersToFrame(float time) {
    // Write in this way so img* could be rvalue
    Image img1 = Image::emptyImage(width, height); 
    Image img2 = renderObject->getFrame(time);

    // Lovely DOUBLE BUFFER WHOOA!
    Image* dest[2];
    bool flag = true;
    dest[0] = &img1;
    dest[1] = &img2;
    for (auto& kv : properties) {
        if (flag) 
            Filter(kv.first).apply(dest[1], dest[0], interpolate(kv.first, time));
        else
            Filter(kv.first).apply(dest[0], dest[1], interpolate(kv.first, time));
        flag = !flag;
    }
    return flag ? *dest[0] : *dest[1];
}

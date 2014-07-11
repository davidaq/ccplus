#include "layer.hpp"

using namespace CCPlus;

Layer::Layer() {}


Layer::Layer(
    Renderable* _renderobject, 
    float _time, 
    float _duration, 
    float _start, 
    float _last
) :
    renderObject(_renderobject),
    time(_time),
    duration(_duration),
    start(_start),
    last(_last)
{
}

Renderable* Layer::getRenderObject() const {
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
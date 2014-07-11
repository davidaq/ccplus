#include "layer.hpp"

using namespace CCPlus;

Layer::Layer() {}


Layer::Layer(
    Context* ctx,
    const std::string& _renderableUri, 
    float _time, 
    float _duration, 
    float _start, 
    float _last
) :
    context(ctx),
    renderableUri(_renderableUri),
    renderObject(0),
    time(_time),
    duration(_duration),
    start(_start),
    last(_last)
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
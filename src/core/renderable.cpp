#include "global.hpp" 

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <cmath>

using namespace CCPlus;

Renderable::Renderable(Context* _ctx, float _dur, int _width, int _height) 
    : context(_ctx), duration(_dur), width(_width), height(_height) 
{
    boost::uuids::uuid u = boost::uuids::random_generator()();
    uuid = boost::lexical_cast<std::string>(u);
}

float Renderable::getDuration() const {
    return duration;
}

int Renderable::getWidth() const {
    return width;
}

int Renderable::getHeight() const {
    return height;
}

int Renderable::getFrameNumber(float time) const {
    float inter = 1.0 / context->getFPS(); 
    return std::round(time / inter);
}

#include "global.hpp" 

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

using namespace CCPlus;

Renderable::Renderable(Context* _ctx, float _dur, float _width, float _height) 
    : context(_ctx), duration(_dur), width(_width), height(_height) 
{
    boost::uuids::uuid u = boost::uuids::random_generator()();
    uuid = boost::lexical_cast<std::string>(u);
}

float Renderable::getDuration() const {
    return duration;
}

float Renderable::getWidth() const {
    return width;
}

float Renderable::getHeight() const {
    return height;
}


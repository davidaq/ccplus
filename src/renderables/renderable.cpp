#include "global.hpp" 

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <cmath>

#ifndef __ANDROID__
#define round(X) std::round(X)
#endif

using namespace CCPlus;

Renderable::Renderable(Context* _ctx) 
    : context(_ctx)
{
    boost::uuids::uuid u = boost::uuids::random_generator()();
    uuid = boost::lexical_cast<std::string>(u);
}

int Renderable::getFrameNumber(float time) const {
    float inter = 1.0 / context->getFPS(); 
    return round(time / inter);
}

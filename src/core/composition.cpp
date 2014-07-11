#include <composition.hpp>

using namespace CCPlus;

Composition::Composition() {}

Composition::Composition(
        CCPlus::Context* ctx, std::string _name,
        float _v, float _dur,
        float _width, float _height) :
    context(ctx), name(_name), version(_v), 
    duration(_dur), width(_width), height(_height)
{
}

std::string Composition::getName() const {
    return "lovely heck!";
}

float Composition::getVersion() const {
    return version;
}

float Composition::getDuration() const {
    return duration;
}

float Composition::getWidth() const {
    return width;
}

float Composition::getHeight() const {
    return height;
}

std::vector<CompositionDependency> Composition::directDependency(float from, float to) const {
    std::vector<CompositionDependency> dps;
    for(Layer layer : getLayers()) {
        if(layer.getTime() < to && layer.getTime() + layer.getDuration() > from) {
            CompositionDependency dp;
            dp.renderable = layer.getRenderObject();
            dp.start = layer.getStart();
            dp.duration = layer.getLast();
            dps.push_back(dp);
        }
    }
    return dps;
}

std::vector<Layer> Composition::getLayers() const {
    std::vector<Layer> layers;
    return layers;
}

#include <composition.hpp>

using namespace CCPlus;

Composition::Composition(
        CCPlus::Context* ctx, std::string _name,
        float _dur, float _width, float _height) :
    Renderable(_dur, _width, _height), context(ctx), name(_name) 
{
}

std::string Composition::getName() const {
    return name;
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
    return layers;
}

void Composition::putLayer(const Layer& layer) {
    layers.push_back(layer);
}



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

std::vector<CompositionDependency> Composition::dependency(float from, float to) const {
    // TODO: calculate dependency
    std::vector<CompositionDependency> dps;
    return dps;
}

std::vector<Layer> Composition::getLayers() const {
    return layers;
}

void Composition::putLayer(const Layer& layer) {
    layers.push_back(layer);
}



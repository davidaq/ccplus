#include <composition.hpp>

using namespace CCPlus;

Composition::Composition() {}

Composition::Composition(CCPlus::Context* ctx) :
    context(ctx)
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

std::vector<CompositionDependency> Composition::dependency(float from, float to) const {
    // TODO: calculate dependency
    std::vector<CompositionDependency> dps;
    return dps;
}

std::vector<Layer> Composition::getLayers() const {
    std::vector<Layer> layers;
    return layers;
}

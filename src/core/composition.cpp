#include <composition.hpp>

using namespace CCPlus;

Composition::Composition() {

}

std::string Composition::getName() const {
    return "lovely heck!";
}

float Composition::getVersion() const {
    return 0.00;
}

float Composition::getDuration() const {
    return 0;
}

float Composition::getWidth() const {
    return 0;
}

float Composition::getHeight() const {
    return 0;
}

std::vector<Composition> Composition::dependency(float from, float to) const {
    // TODO: calculate dependency
    std::vector<Composition> dps;
    return dps;
}

std::vector<Layer> Composition::getLayers() const {
    std::vector<Layer> layers;
    return layers;
}

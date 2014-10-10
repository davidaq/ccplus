#include "composition.hpp"
#include "layer.hpp"

using namespace CCPlus;

Composition::Composition(const std::string& name, float duration, int width, int height) {
    this->name = name;
    this->duration = duration;
    this->width = width;
    this->height = height;
}

void Composition::appendLayer(const Layer& layer) {
    layers.push_back(layer);
}

Frame Composition::getFrame(int time) {
    return Frame();
}

void Composition::prepare() {
    
}

int Composition::getDuration() {
    return duration;
}

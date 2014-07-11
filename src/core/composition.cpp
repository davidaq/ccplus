#include <composition.hpp>

using namespace CCPlus;

Composition::Composition() :
    context(0)
{}

Composition::Composition(CCPlus::Context* ctx) :
    context(ctx)
{
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

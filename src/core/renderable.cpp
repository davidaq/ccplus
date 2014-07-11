#include "global.hpp" 

using namespace CCPlus;

Renderable::Renderable(float _dur, float _width, float _height) 
    : duration(_dur), width(_width), height(_height) 
{}

float Renderable::getDuration() const {
    return duration;
}

float Renderable::getWidth() const {
    return width;
}

float Renderable::getHeight() const {
    return height;
}


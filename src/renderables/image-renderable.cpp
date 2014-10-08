#include <limits>
#include <iostream>

#include "image-renderable.hpp"
#include "utils.hpp"
#include "context.hpp"

using namespace CCPlus;

ImageRenderable::ImageRenderable(const std::string& _uri) :
    Renderable(),
    uri(_uri)
{
    path = uri;
    if (stringStartsWith(path, "file://")) 
        path = uri.substr(7);
    path = generatePath(context->getInputDir(), path);
}

// Legacy deprecated
int ImageRenderable::getWidth() const {
    return 0;
}

// Legacy deprecated
int ImageRenderable::getHeight() const {
    return 0;
}

float ImageRenderable::getDuration() const {
    return std::numeric_limits<int>::max();
}

void ImageRenderable::clear() {
}

void ImageRenderable::render(float start, float duration) {
    cache = Frame(path);
}

Frame ImageRenderable::getFrame(float time) const {
    return Frame(cache.getImage().clone());
}

Frame ImageRenderable::getFrameByNumber(int frame) const {
    return Frame(cache.getImage().clone());
}

const std::string& ImageRenderable::getName() const {
    return uri;
}

bool ImageRenderable::still(float t1, float t2) {
    return true;
}

std::string ImageRenderable::getFramePath(int f) const {
    return path;
}

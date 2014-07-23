#include "image-renderable.hpp"
#include <limits>
#include "utils.hpp"
#include <iostream>

using namespace CCPlus;

ImageRenderable::ImageRenderable(Context* context, const std::string& uri) :
    Renderable(context) 
{
    std::string path = uri;
    if (stringStartsWith(path, "file://")) 
        path = uri.substr(7);
    image = Image(path);
}

int ImageRenderable::getWidth() const {
    return image.getWidth();
}

int ImageRenderable::getHeight() const {
    return image.getHeight();
}

float ImageRenderable::getDuration() const {
    return std::numeric_limits<int>::max();
}

void ImageRenderable::render(float start, float duration) {
    // Nothing shall be done
}

Image ImageRenderable::getFrame(float time) const {
    return image;
}

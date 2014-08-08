#include <limits>
#include <iostream>

#include "image-renderable.hpp"
#include "utils.hpp"
#include "context.hpp"

using namespace CCPlus;

ImageRenderable::ImageRenderable(Context* context, const std::string& _uri) :
    Renderable(context),
    uri(_uri)
{
    path = uri;
    if (stringStartsWith(path, "file://")) 
        path = uri.substr(7);
    path = generatePath(context->getInputDir(), path);
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
    //if (!loaded)
    //    image = Frame(path);
    //loaded = true;
    cache = Frame(path);
}

Frame ImageRenderable::getFrame(float time) const {
    return Frame(cache.getImage().clone());
}

const std::string& ImageRenderable::getName() const {
    return uri;
}

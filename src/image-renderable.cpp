#include "image-renderable.hpp"

using namespace CCPlus;

ImageRenderable::ImageRenderable(const std::string& uri) {
    this->uri = uri;
}

Frame ImageRenderable::getFrame(float) {
    return image;
}

float ImageRenderable::getDuration() {
    return 1;
}

void ImageRenderable::prepare() {
    image.image = cv::imread(parseUri2File(uri));
}


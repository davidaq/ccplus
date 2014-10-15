#include "image-renderable.hpp"
#include "gpu-frame-impl.hpp"
#include "gpu-frame-cache.hpp"

using namespace CCPlus;
using namespace cv;

ImageRenderable::ImageRenderable(const std::string& uri) {
    this->uri = uri;
}

GPUFrame ImageRenderable::getGPUFrame(float) {
    GPUFrame frame = GPUFrameCache::alloc(image.image.cols, image.image.rows);
    frame->load(image);
    return frame;
}

float ImageRenderable::getDuration() {
    return 100000;
}

void ImageRenderable::prepare() {
    std::string filepath = parseUri2File(uri);
    Mat org = cv::imread(filepath);
    mat3to4(org);

    auto rotateCWRightAngle = [&org] (int angle) {
        if (angle % 90 != 0) {
            log(logWARN) << "Unexpected rotation angle: " << angle;
        }

        if (angle == 180) {
            flip(org, org, -1); 
        } else if (angle == 90) {
            transpose(org, org);
            flip(org, org, 1); 
        } else if (angle == 270) {
            transpose(org, org);
            flip(org, org, 0); 
        }
    }; 

    if (stringEndsWith(toLower(filepath), ".jpg")) {
        try {
            int degree = getImageRotation(filepath);
            if (degree == -1) {
                log(logWARN) << "Failed getting image rotationg angle";
            } else {
                rotateCWRightAngle(degree);
            }
        } catch (...) {
            log(logWARN) << "Failed getting image rotationg angle";
        }
    }

    image.image = org;
}

void ImageRenderable::release() {
    image.image = cv::Mat();
}

int ImageRenderable::getWidth() const {
    return image.image.cols;
}

int ImageRenderable::getHeight() const {
    return image.image.rows;
}


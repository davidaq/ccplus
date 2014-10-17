#include "image-renderable.hpp"
#include "gpu-frame-impl.hpp"
#include "gpu-frame-cache.hpp"

using namespace CCPlus;
using namespace cv;

ImageRenderable::ImageRenderable(const std::string& uri) {
    this->uri = uri;
}

GPUFrame ImageRenderable::getGPUFrame(float) {
    if(!gpuCache) {
        gpuCache = GPUFrameCache::alloc(image.image.cols, image.image.rows);
        gpuCache->load(image);
    }
    return gpuCache;
}

float ImageRenderable::getDuration() {
    return 100000;
}

void ImageRenderable::prepare() {
    std::string filepath = parseUri2File(uri);
    Mat org = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);

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

    int w = org.cols, h = org.rows;
    float max_size = 1280.0;
    if(w > max_size) {
        h *= max_size / w;
        w = max_size;
    }
    if(h > max_size) {
        w *= max_size / h;
        h = max_size;
    }
    if(w != org.cols) {
        int x = org.cols;
        int y = org.rows;
        cv::resize(org, org, {w, h}, 0, 0, INTER_AREA);
        image.ext.scaleAdjustX = x * 1.0f / org.cols;
        image.ext.scaleAdjustY = y * 1.0f / org.rows;
    }

    image.image = org;
}

void ImageRenderable::release() {
    image.image = cv::Mat();
    gpuCache = GPUFrame();
}

int ImageRenderable::getWidth() const {
    return image.image.cols;
}

int ImageRenderable::getHeight() const {
    return image.image.rows;
}


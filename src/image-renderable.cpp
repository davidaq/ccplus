#include "image-renderable.hpp"
#include "gpu-frame-impl.hpp"
#include "gpu-frame-cache.hpp"
#include "ccplus.hpp"

using namespace CCPlus;
using namespace cv;

ImageRenderable::ImageRenderable(const std::string& uri) {
    this->uri = uri;
}

GPUFrame ImageRenderable::getGPUFrame(float) {
    if(!gpuCache) {
        Frame im = image.decompressed();
        gpuCache = GPUFrameCache::alloc(im.image.cols, im.image.rows);
        gpuCache->load(im);
    }
    return gpuCache;
}

float ImageRenderable::getDuration() {
    return 100000;
}

void ImageRenderable::prepare() {
    if (prepared) {
        return;
    }
    prepared = true;
    std::string filepath = parseUri2File(uri);
    Mat org = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
    if (!org.data) {
        log(logERROR) << "Could not open or find the image: " << filepath;
        return;
    }

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

    image.toNearestPOT(renderMode == PREVIEW_MODE ? 512 : 1024);

#ifdef __ANDROID__
    cv::cvtColor(image.image, image.image, CV_BGRA2RGBA);
#endif
    image = image.compressed();
}

void ImageRenderable::release() {
    image.image = cv::Mat();
    gpuCache = GPUFrame();
}

void ImageRenderable::releaseGPUCache() {
    gpuCache = GPUFrame();
}

int ImageRenderable::getWidth() const {
    return image.image.cols;
}

int ImageRenderable::getHeight() const {
    return image.image.rows;
}


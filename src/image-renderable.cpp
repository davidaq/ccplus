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
        gpuCache = image.toGPU();
    }
    return gpuCache;
}

float ImageRenderable::getDuration() {
    return 100000;
}

void ImageRenderable::prepareWithFileData(const uint8_t* data, uint32_t len, int rot) {
    Mat org = imdecode(std::vector<uint8_t>(data, data + len), CV_LOAD_IMAGE_UNCHANGED);
    if (!org.data) {
        log(logERROR) << "Could not open or find the image: " << uri;
        return;
    }
    mat3to4(org);
    int degree = 0;
    try {
        degree = getImageRotation(data, len);
        if (degree == -1) {
            throw "fail";
        } 
    } catch (...) {
        degree = 0;
        log(logWARN) << "Failed getting image rotationg angle";
    }
    try {
        degree += rot;
        if(degree != 0) {
            int angle = degree;
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
        }
    } catch (...) {
        log(logWARN) << "Failed rotating image";
    }
    image.image = org;

    image.toNearestPOT(renderMode == PREVIEW_MODE ? 512 : 1024);

#ifdef __ANDROID__
    cv::cvtColor(image.image, image.image, CV_BGRA2RGBA);
#endif
    image = image.compressed();
}

#ifdef __IOS__
void prepareWithFileData(void* ctx, const uint8_t* data, uint32_t len) {
    ((ImageRenderable*)ctx)->prepareWithFileData(data, len);
}

#endif
void ImageRenderable::prepare() {
    gpuCache = GPUFrame();
    int rotation = 0;
    std::string filepath = parseUri2File(uri, &rotation);
#ifdef __IOS__
    getAssetsLibraryImage(filepath.c_str(), this, ::prepareWithFileData);
    if(!image.image.empty())
        return;
#endif
    FILE* fp = fopen(filepath.c_str(), "rb");
    if(!fp) 
        return;
    fseek(fp, 0, SEEK_END);
    uint32_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t* data = new uint8_t[len];
    fread(data, 1, len, fp);
    fclose(fp);
    prepareWithFileData(data, len, rotation);
    delete[] data;
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


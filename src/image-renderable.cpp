#include "image-renderable.hpp"

using namespace CCPlus;
using namespace cv;

ImageRenderable::ImageRenderable(const std::string& uri) {
    this->uri = uri;
}

Frame ImageRenderable::getFrame(float) {
    return image;
}

float ImageRenderable::getDuration() {
    return 100000;
}

void ImageRenderable::prepare() {
    std::string filepath = parseUri2File(uri);
    Mat org = cv::imread(filepath);
    if (org.channels() == 3) {
        cv::Mat newimg = cv::Mat(org.rows, org.cols, CV_8UC4, {0, 0, 0, 255});
        int from_to[] = {0, 0, 1, 1, 2, 2};
        mixChannels(&org, 1, &newimg, 1, from_to, 3);
        org = newimg;
    }

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
}

int ImageRenderable::getWidth() const {
    return image.image.cols;
}

int ImageRenderable::getHeight() const {
    return image.image.rows;
}


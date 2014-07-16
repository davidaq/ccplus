#include "global.hpp"
#include <ios>
#include <stdexcept>

using namespace CCPlus;
using namespace cv;

Image::Image(const char* filepath) {
    data = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
    if (!data.data) {
        throw std::ios_base::failure("File not exists");
    }
}

void Image::write(const char* file) {

}

int Image::getWidth() const {
    return data.cols;
}

int Image::getHeight() const {
    return data.rows;
}

int Image::getChannels() const {
    return data.channels();
}

cv::Mat* Image::getData() {
    return &data;
}

const cv::Mat* Image::getData() const {
    return &data;
}

/**
 * Put img on top of this image
 * REQUIRE: img must be a RGBA imaga
 */
void Image::overlayImage(const Image* input) {
    const Mat* img = input->getData();
    if (this->getHeight() != input->getHeight() || this->getWidth() != input->getWidth()) {
        throw std::invalid_argument("overlayImage: images have to be with the same size");
    }

    if (this->getChannels() != 4 || input->getChannels() != 4) {
        throw std::invalid_argument("overlayImage: images have to have alpha channels");
    }

    //std::cout << (float) img->at<Vec3b>(1079, 1692)[0] << std::endl;
    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            //std::cout << i << " " << j << std::endl;
            uchar alpha_this = data.at<Vec4b>(i, j)[3];
            uchar alpha_img = img->at<Vec4b>(i, j)[3];
            float falpha_this = (float) (alpha_this / 255);
            float falpha_img = (float) (alpha_img / 255);
            float fnew_alpha = falpha_this + (1.0 - falpha_this) * falpha_img;
            for (int k = 0; k < 3; k++) {
                float x = (float) data.at<Vec4b>(i, j)[k];
                float y = (float) img->at<Vec4b>(i, j)[k];
                float ret = falpha_this * x + (1 - falpha_this) * falpha_img * y;
                data.at<Vec4b>(i, j)[k] = (uchar) ret;
            }
            data.at<Vec4b>(i, j)[3] = (uchar) (255 * fnew_alpha);
        }
    }
}

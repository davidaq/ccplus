#include "global.hpp"
#include "stringutil.hpp"
#include <ios>
#include <stdexcept>

using namespace CCPlus;
using namespace cv;

Image::Image(const std::string& filepath) {
    if(stringEndsWith(filepath, ".zim")) {
        // read zipped argb data
    } else {
        // read from file system
        data = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
        if (!data.data) {
            throw std::ios_base::failure("File not exists");
        }
        // TODO take EXIF into account
    }
}

Image::Image() {
}

void Image::write(const char* file) {

}

void Image::write(const std::string& file) {
    
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

// Only for testing ;
void Image::setData(const cv::Mat& m) {
    this->data = m;
}

/**
 * Put img *UNDER* this image
 * REQUIRE: img must be a RGBA image
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
            float falpha_this = alpha_this / 255.0;
            float falpha_img = alpha_img / 255.0;
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

Image Image::emptyImage(int width, int height) {
    Mat m = Mat::zeros(height, width, CV_8UC4);
    Image ret;
    ret.setData(m);
    return ret;
}

/**
 * Eliminate alpha, set transparent parts to white
 */
void Image::setWhiteBackground() {
    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            int alpha = data.at<Vec4b>(i, j)[3];
            for(int k = 0; k < 3; k++) {
                data.at<Vec4b>(i, j)[k] = alpha * data.at<Vec4b>(i, j)[k] / 0xff + (0xff - alpha);
            }
        }
    }
}

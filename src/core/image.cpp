#include "global.hpp"
#include <ios>

using namespace CCPlus;

Image::Image(const char* filepath) {
    data = cv::imread(filepath);
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

cv::Mat Image::getData() {
    return data;
}

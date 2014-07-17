#include "global.hpp"
#include "util.hpp"
#include "zip.hpp"
#include <ios>
#include <stdexcept>

using namespace CCPlus;
using namespace cv;

Image::Image(const std::string& filepath) {
    if(stringEndsWith(filepath, ".zim")) {
        FILE* inFile = fopen(filepath.c_str(), "rb");       
        if(!inFile)
            throw std::ios_base::failure("File not exists");
        fseek(inFile, 0, SEEK_END);       
        size_t len = ftell(inFile);
        if(len < 5) {
            fclose(inFile);
            throw std::ios_base::failure("File too small to be a image");
        }       
        fseek(inFile, 0, SEEK_SET);       
        unsigned char* compressedBytes = new unsigned char[len];
        fread(compressedBytes, sizeof(char), len, inFile);       
        fclose(inFile);
        unsigned short width = *((unsigned short*)compressedBytes);
        unsigned short height = *((unsigned short*)(compressedBytes + sizeof(unsigned short))); 
        unsigned char* bgraBytes = new unsigned char[width * height * 4];       
        unsigned long destLen = width * height * 4;
        uncompress(bgraBytes, &destLen, compressedBytes + sizeof(unsigned short) * 2, len - sizeof(unsigned short) * 2);
        if(destLen != width * height * 4) {
            throw std::ios_base::failure("Unexpected uncompressed length");       
        }
        data = cv::Mat(height, width, CV_8UC4, bgraBytes);       
        delete[] bgraBytes;
        delete[] compressedBytes;
    } else {
        // read from file system
        data = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
        if (!data.data) {
            throw std::ios_base::failure("File not exists");
        }
        // TODO take EXIF into account
    }

    if (data.channels() == 3) {
        to4Channels();
    } else if (data.channels() < 3) {
        throw std::invalid_argument("Can't take image with less than 3 channels");
    }
}

Image::Image() {
}

void Image::to4Channels() {
    Mat newdata = Mat(getHeight(), getWidth(), CV_8UC4, {0, 0, 0, 255});
    int from_to[] = {0, 0, 1, 1, 2, 2};
    mixChannels(&data, 1, &newdata, 1, from_to, 3);
    data = newdata;
}

void Image::write(const std::string& file) {
    if(stringEndsWith(file, ".zim")) {   
        FILE* outFile = fopen(file.c_str(), "wb");       
        if(!outFile)
            throw std::ios_base::failure("File path [" + file + "] unwritable");   
        unsigned short metric; 
        metric = (unsigned short)getWidth();
        fwrite(&metric, sizeof(metric), 1, outFile);
        metric = (unsigned short)getHeight();
        fwrite(&metric, sizeof(metric), 1, outFile);
        unsigned char* compressedBytes = new unsigned char[data.cols * data.rows * 4];
        unsigned long len = data.cols * data.rows * 4;       
        compress(compressedBytes, &len, data.data, data.cols * data.rows * 4);
        fwrite(compressedBytes, sizeof(unsigned char), len, outFile);
        fclose(outFile);       
        delete[] compressedBytes;
    } else {
        cv::imwrite(file, data);
    }
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

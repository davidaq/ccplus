#include "global.hpp"
#include "utils.hpp"
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
        unsigned char* fileContent = new unsigned char[len];
        fread(fileContent, sizeof(char), len, inFile);       
        fclose(inFile);
        unsigned char* ptr = fileContent;
#define NEXT(TYPE) *((TYPE*)ptr);ptr += sizeof(TYPE)
        unsigned short width = NEXT(unsigned short);
        unsigned short height = NEXT(unsigned short); 
        
        unsigned long jpgLen = NEXT(unsigned long);
        vector<unsigned char> jpgBuff(ptr, ptr + jpgLen);
        data = cv::imdecode(jpgBuff, CV_LOAD_IMAGE_COLOR);
        to4Channels();
        ptr += jpgLen;

        unsigned alphaLen = NEXT(unsigned long);
        unsigned long destLen = width * height;
        unsigned char* alphaBytes = new unsigned char[destLen];       
        uncompress(alphaBytes, &destLen, ptr, alphaLen);
        if(destLen != width * height) {
            throw std::ios_base::failure("Unexpected uncompressed length");       
        }
        for(int i = 3, j =0; j < destLen; i += 4, j++) {
            data.data[i] = alphaBytes[j];
        }
        delete[] alphaBytes;
        delete[] fileContent;
    } else {
        // read from file system
        data = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
        if (!data.data) {
            throw std::ios_base::failure("File not exists");
        }
    }

    if (data.channels() == 3) {
        to4Channels();
    } else if (data.channels() < 3) {
        throw std::invalid_argument("Can't take image with less than 3 channels");
    }

    if (stringEndsWith(toLower(filepath), ".jpg")) {
        try {
            int degree = getImageRotation(filepath);
            rotateCWRightAngle(degree);
        } catch (...) {
            std::cout << "Something wrong about rotation" << std::endl;
        }
    }
}

Image::Image(int width, int height) {
    data = Mat::zeros(height, width, CV_8UC4);
}

Image::Image(const cv::Mat& _data) {
    data = _data;
}

Image::Image() {
}

void Image::rotateCWRightAngle(int angle) {
    if (angle % 90 != 0)
        throw std::invalid_argument("Only allowed right angle rotation 90, 180, 270");

    if (angle == 180) {
        flip(data, data, -1); 
    } else if (angle == 90) {
        transpose(data, data);
        flip(data, data, 1); 
    } else if (angle == 270) {
        transpose(data, data);
        flip(data, data, 0); 
    }
}

void Image::to4Channels() {
    Mat newdata = Mat(getHeight(), getWidth(), CV_8UC4, {0, 0, 0, 255});
    int from_to[] = {0, 0, 1, 1, 2, 2};
    mixChannels(&data, 1, &newdata, 1, from_to, 3);
    data = newdata;
}

void Image::write(const std::string& file, int quality) {
    if(stringEndsWith(file, ".zim")) {   
        FILE* outFile = fopen(file.c_str(), "wb");       
        if(!outFile)
            throw std::ios_base::failure("File path [" + file + "] unwritable");   
        unsigned short metric; 
        // write size first
        metric = (unsigned short)getWidth();
        fwrite(&metric, sizeof(metric), 1, outFile);
        metric = (unsigned short)getHeight();
        fwrite(&metric, sizeof(metric), 1, outFile);

        // write jpeg encoded color part
        vector<unsigned char> buff;
        imencode(".jpg", data, buff, vector<int>{CV_IMWRITE_JPEG_QUALITY, 80});
        unsigned long jpgLen = buff.size();
        fwrite(&jpgLen, sizeof(long), 1, outFile);
        fwrite(&buff[0], sizeof(char), jpgLen, outFile);
        buff.clear();

        // write zip compressed alpha channel
        unsigned long len = data.cols * data.rows;
        unsigned char* uncompressedBytes = new unsigned char[len];
        unsigned char* compressedBytes = new unsigned char[len];
        for(int i = 3, j = 0, c = len * 4; i < c; i += 4, j++) {
            uncompressedBytes[j] = data.data[i];
        }
        compress(compressedBytes, &len, uncompressedBytes, len);
        fwrite(&len, sizeof(long), 1, outFile);
        fwrite(compressedBytes, sizeof(unsigned char), len, outFile);
        fclose(outFile);       
        delete[] uncompressedBytes;
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

cv::Mat& Image::getData() {
    return data;
}

const cv::Mat& Image::getData() const {
    return data;
}

void Image::setData(const cv::Mat& m) {
    this->data = m;
}

/**
 * Put img *UNDER* this image
 * REQUIRE: img must be a RGBA image
 */
void Image::overlayImage(const Image& input) {
    const Mat& img = input.getData();
    if (this->getHeight() != input.getHeight() || this->getWidth() != input.getWidth()) {
        throw std::invalid_argument("overlayImage: images have to be with the same size");
    }

    if (this->getChannels() != 4 || input.getChannels() != 4) {
        throw std::invalid_argument("overlayImage: images have to have alpha channels");
    }

    //std::cout << (float) img.at<Vec3b>(1079, 1692)[0] << std::endl;
    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            //std::cout << i << " " << j << std::endl;
            uchar alpha_this = data.at<Vec4b>(i, j)[3];
            uchar alpha_img = img.at<Vec4b>(i, j)[3];
            float falpha_this = alpha_this / 255.0;
            float falpha_img = alpha_img / 255.0;
            float fnew_alpha = falpha_this + (1.0 - falpha_this) * falpha_img;
            for (int k = 0; k < 3; k++) {
                float x = (float) data.at<Vec4b>(i, j)[k];
                float y = (float) img.at<Vec4b>(i, j)[k];
                float ret = falpha_this * x + (1 - falpha_this) * falpha_img * y;
                data.at<Vec4b>(i, j)[k] = (uchar) ret;
            }
            data.at<Vec4b>(i, j)[3] = (uchar) (255 * fnew_alpha);
        }
    }
}

Image Image::emptyImage(int width, int height) {
    return Image(width, height);
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

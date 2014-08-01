#include "global.hpp"
#include "utils.hpp"
#include "zip.hpp"
#include <ios>
#include <stdexcept>
#include <algorithm>

using namespace CCPlus;
using namespace cv;

typedef uint32_t ulong;
typedef uint16_t ushort;

Frame::Frame(const std::string& filepath) {
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
        ushort width = NEXT(ushort);
        ushort height = NEXT(ushort); 
        
        ulong jpgLen = NEXT(ulong);
        if (jpgLen >= 125) {
            vector<unsigned char> jpgBuff(ptr, ptr + jpgLen);
            image = cv::imdecode(jpgBuff, CV_LOAD_IMAGE_COLOR);
            to4Channels();
        }
        ptr += jpgLen;

        ulong alphaLen = NEXT(ulong);
        unsigned long destLen = width * height;
        unsigned char* alphaBytes = new unsigned char[destLen];       
        uncompress(alphaBytes, &destLen, ptr, alphaLen);
        if(destLen != width * height) {
            throw std::ios_base::failure("Unexpected uncompressed length");       
        }
        for(int i = 3, j =0; j < destLen; i += 4, j++) {
            image.data[i] = alphaBytes[j];
        }
        ptr += alphaLen;

        ulong audioLen = NEXT(ulong);
        ulong audioRealByteLen = NEXT(ulong);
        unsigned char* audioData = new unsigned char[audioRealByteLen];
        destLen = (unsigned long)0x7fffffff;
        int ret = uncompress(audioData, &destLen, ptr, audioLen);
        if (ret != 0) 
            throw std::ios_base::failure("Uncompress failed");
        
        // I don't trust memcpy
        std::vector<int16_t> tmp;
        for (int i = 0; i < destLen / 2; i++) 
            tmp.push_back(((int16_t*) audioData)[i]);
        audio = Mat(tmp, true);
        
        delete[] alphaBytes;
        delete[] fileContent;
        delete[] audioData;
    } else {
        // read from file system
        // ignore audio
        image = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
        if (!image.data) {
            throw std::ios_base::failure("File not exists");
        }
    }

    if (image.channels() == 3) {
        to4Channels();
    } else if (image.channels() < 3 && !image.empty()) {
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

Frame::Frame(int width, int height) {
    image = Mat::zeros(height, width, CV_8UC4);
}

Frame::Frame(const cv::Mat& _data) {
    if (_data.type() == CV_16S) {
        audio = _data;
    } else if (_data.type() == CV_8UC4) {
        image = _data;
    }
}

Frame::Frame(const std::vector<int16_t>& data) {
    audio = Mat(data);
}

Frame::Frame(const cv::Mat& _image, const cv::Mat& _audio) : image(_image), audio(_audio) {};

Frame::Frame() {}

void Frame::rotateCWRightAngle(int angle) {
    if (angle % 90 != 0)
        throw std::invalid_argument("Only allowed right angle rotation 90, 180, 270");

    if (angle == 180) {
        flip(image, image, -1); 
    } else if (angle == 90) {
        transpose(image, image);
        flip(image, image, 1); 
    } else if (angle == 270) {
        transpose(image, image);
        flip(image, image, 0); 
    }
}

void Frame::to4Channels() {
    Mat newdata = Mat(getHeight(), getWidth(), CV_8UC4, {0, 0, 0, 255});
    int from_to[] = {0, 0, 1, 1, 2, 2};
    mixChannels(&image, 1, &newdata, 1, from_to, 3);
    image = newdata;
}

void Frame::write(const std::string& file, int quality) {
    if(stringEndsWith(file, ".zim")) {   
        FILE* outFile = fopen(file.c_str(), "wb");       
        if(!outFile) {
            throw std::ios_base::failure("File path [" + file + "] unwritable");   
        }
        ushort metric; 
        // write size first
        metric = (ushort)getWidth();
        fwrite(&metric, sizeof(metric), 1, outFile);
        metric = (ushort)getHeight();
        fwrite(&metric, sizeof(metric), 1, outFile);

        // write jpeg encoded color part
        vector<unsigned char> buff;
        if (!image.empty()) {
            imencode(".jpg", image, buff, vector<int>{CV_IMWRITE_JPEG_QUALITY, quality});
        }
        ulong jpgLen = buff.size();
        fwrite(&jpgLen, sizeof(jpgLen), 1, outFile);
        if (!image.empty())
            fwrite(&buff[0], sizeof(char), jpgLen, outFile);
        buff.clear();

        // write zip compressed alpha channel
        unsigned long len = image.cols * image.rows;
        unsigned char* uncompressedBytes = new unsigned char[len];
        unsigned char* compressedBytes = new unsigned char[len];
        for(int i = 3, j = 0, c = len * 4; i < c; i += 4, j++) 
            uncompressedBytes[j] = image.data[i];
        compress(compressedBytes, &len, uncompressedBytes, len);
        ulong wlen = len;
        fwrite(&wlen, sizeof(wlen), 1, outFile);
        fwrite(compressedBytes, sizeof(unsigned char), len, outFile);

        // Compress audio data
        len = audio.total();
        //unsigned long tmp = std::max(len * 2 + 128, (unsigned long)128);
        unsigned long tmp = len * 2 + 128;
        unsigned char* compressedAudio = new unsigned char[tmp];
        int ret = compress(compressedAudio, &tmp, (unsigned char*) audio.data, len * 2);
        if (ret != 0) {
            throw std::ios_base::failure("Compressing audio failed: returned " + std::to_string(ret));
        }
        // ulong is NOT unsigned long
        wlen = tmp;
        fwrite(&wlen, sizeof(wlen), 1, outFile);
        wlen = len * 2;
        fwrite(&wlen, sizeof(wlen), 1, outFile);
        fwrite(compressedAudio, sizeof(unsigned char), tmp, outFile);

        fclose(outFile);
        delete[] uncompressedBytes;
        delete[] compressedBytes;
        delete[] compressedAudio;
    } else {
        cv::imwrite(file, image);
    }
}

int Frame::getWidth() const {
    return image.cols;
}

int Frame::getHeight() const {
    return image.rows;
}

int Frame::getImageChannels() const {
    return image.channels();
}

cv::Mat& Frame::getImage() {
    return image;
}

const cv::Mat& Frame::getImage() const {
    return image;
}

void Frame::setImage(const cv::Mat& m) {
    this->image = m;
}

const Mat& Frame::getAudio() const {
    return audio;
}

Mat& Frame::getAudio() {
    return audio;
}

void Frame::setAudio(const Mat& aud) {
    this->audio = aud;
}

void Frame::setAudio(const std::vector<int16_t>& aud) {
    this->audio = Mat(aud, true);
}

void Frame::mergeFrame(const Frame& f) {
    if (!f.getImage().empty())
        this->overlayImage(f.getImage());
    // Merge audio
    // Two frames are supposed to have the same number of audio signals
    const Mat& input = f.getAudio();
    if (audio.total() == 0) {
        audio = input;
        return;
    }
    for (int i = 0; i < input.total() && i < audio.total(); i++) {
        int tmp = (int)audio.at<int16_t>(i) + (int)input.at<int16_t>(i);
        tmp = std::max((int)std::numeric_limits<int16_t>::min(), tmp);
        tmp = std::min((int)std::numeric_limits<int16_t>::max(), tmp);
        audio.at<int16_t>(i) = int16_t(tmp);
    }
}

void Frame::overlayImage(const cv::Mat& input) {
    if (this->getHeight() != input.rows || this->getWidth() != input.cols) {
        throw std::invalid_argument("overlayFrame: images have to be with the same size");
    }

    if (this->getImageChannels() != 4 || input.channels() != 4) {
        throw std::invalid_argument("overlayFrame: images have to have alpha channels");
    }

    //std::cout << (float) img.at<Vec3b>(1079, 1692)[0] << std::endl;
    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            //std::cout << i << " " << j << std::endl;
            uchar alpha_this = image.at<Vec4b>(i, j)[3];
            uchar alpha_img = input.at<Vec4b>(i, j)[3];
            float falpha_this = alpha_this / 255.0;
            float falpha_img = alpha_img / 255.0;
            float fnew_alpha = falpha_this + (1.0 - falpha_this) * falpha_img;
            for (int k = 0; k < 3; k++) {
                float x = (float) image.at<Vec4b>(i, j)[k];
                float y = (float) input.at<Vec4b>(i, j)[k];
                float ret = falpha_this * x + (1 - falpha_this) * falpha_img * y;
                image.at<Vec4b>(i, j)[k] = (uchar) ret;
            }
            image.at<Vec4b>(i, j)[3] = (uchar) (255 * fnew_alpha);
        }
    }
}

Frame Frame::emptyFrame(int width, int height) {
    return Frame(width, height);
}

/**
 * Eliminate alpha, set transparent parts to white
 */
void Frame::setWhiteBackground() {
    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            int alpha = image.at<Vec4b>(i, j)[3];
            for(int k = 0; k < 3; k++) {
                image.at<Vec4b>(i, j)[k] = alpha * image.at<Vec4b>(i, j)[k] / 0xff + (0xff - alpha);
            }
        }
    }
}

#include "global.hpp"
#include "utils.hpp"
#include "file-manager.hpp"
#include "file.hpp"
#include "zip.hpp"
#include <ios>
#include <stdexcept>
#include <algorithm>
#include "file.hpp"
#include "file-manager.hpp"

using namespace CCPlus;
using namespace cv;

typedef uint32_t ulong;
typedef uint16_t ushort;

Frame::Frame(const std::string& filepath) {
    if(stringEndsWith(filepath, ".zim")) {
        File* inFile = FileManager::getInstance()->open(filepath, "rb");
        if(!inFile)
        {
            log(logFATAL) << "Intermidiate file not exists: " << filepath;
        }
        unsigned char* fileContent = new unsigned char[inFile->getSize()];
        inFile->readAll(fileContent);
        inFile->close();
        unsigned char* ptr = fileContent;
#define NEXT(TYPE) *((TYPE*)ptr);ptr += sizeof(TYPE)

        // Image size 
        ushort width = NEXT(ushort);
        ushort height = NEXT(ushort); 

        /* 
         * Read jpg part
         */
        ulong jpgLen = NEXT(ulong);
        if (jpgLen >= 125) {
            vector<unsigned char> jpgBuff(ptr, ptr + jpgLen);
            profile(DecodeImage) {
                image = cv::imdecode(jpgBuff, CV_LOAD_IMAGE_COLOR);
            }
            to4Channels();
        }
        ptr += jpgLen;

        /*
         * Read alpha channel
         */
        ulong alphaLen = NEXT(ulong);
        unsigned long destLen = width * height;
        unsigned char* alphaBytes = new unsigned char[destLen];       
        int ret = 0;
        profile(ZipUncompress) {
            ret = uncompress(alphaBytes, &destLen, ptr, alphaLen);
        }
        if(destLen != width * height || ret != 0) {
            log(logFATAL) << "Failed decompressing alpha";
        }
        for(int i = 3, j =0; j < destLen; i += 4, j++) {
            image.data[i] = alphaBytes[j];
        }
        ptr += alphaLen;

        /*
         * Read audio channel
         */
        ulong audioLen = NEXT(ulong);
        ulong audioRealByteLen = NEXT(ulong);
        if (CCPlus::COMPRESS_AUDIO) {  
            unsigned char* audioData = new unsigned char[audioRealByteLen];
            destLen = (unsigned long)0x7fffffff;
            profile(ZipUncompress) {
                ret = uncompress(audioData, &destLen, ptr, audioLen);
            }
            if (ret != 0 || audioRealByteLen != destLen) {
                log(logFATAL) << "Failed decompressing audio";
            }
            // I don't trust memcpy
            std::vector<int16_t> tmp;
            for (int i = 0; i < destLen / 2; i++) 
                tmp.push_back(((int16_t*) audioData)[i]);
            audio = Mat(tmp, true);
            delete[] audioData;
        } else {
            vector<int16_t> tmp((int16_t*)ptr, (int16_t*)ptr + audioRealByteLen / 2);   
            audio = Mat(tmp, true);
        }        
        delete[] alphaBytes;
        delete[] fileContent;
    } else {
        // read from file system
        // ignore audio
        image = cv::imread(filepath, CV_LOAD_IMAGE_UNCHANGED);
        if (!image.data) {
            log(logFATAL) << "file not exists: " << filepath;
            //throw std::ios_base::failure("File not exists");
        }
    }

    if (image.channels() == 3) {
        to4Channels();
    } else if (image.channels() < 3 && !image.empty()) {
        log(logFATAL) << "Can't take images with less than 3 channels";
    }

    if (stringEndsWith(toLower(filepath), ".jpg")) {
        try {
            int degree = getImageRotation(filepath);
            if (degree == -1) {
                log(logWARN) << "Failed getting image rotationg angle";
                return;
            }
            rotateCWRightAngle(degree);
        } catch (...) {
            log(logWARN) << "Failed getting image rotationg angle";
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
    audio = Mat(data, true);
}

Frame::Frame(const cv::Mat& _image, const cv::Mat& _audio) : image(_image), audio(_audio) {};

Frame::Frame() {}

void Frame::rotateCWRightAngle(int angle) {
    if (angle % 90 != 0) {
        log(logWARN) << "Unexpected rotation angle: " << angle;
    }

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

void Frame::write(const std::string& file, int quality, bool inMemory) {
    if(stringEndsWith(file, ".zim")) {   
        FileManager* fm = FileManager::getInstance();
        File& outFile = *fm->open(file, "wb", inMemory);
        ushort metric; 
        // write size first
        metric = (ushort)getWidth();
        outFile.write(&metric, sizeof(metric));
        metric = (ushort)getHeight();
        outFile.write(&metric, sizeof(metric));

        /* 
         * write jpeg encoded color part
         */
        vector<unsigned char> buff;
        if (!image.empty()) {
            profile(EncodeImage) {
                imencode(".jpg", image, buff, 
                        vector<int>{CV_IMWRITE_JPEG_QUALITY, quality});
            }
        }
        ulong jpgLen = buff.size();
        outFile.write(&jpgLen, sizeof(jpgLen));
        if (!image.empty())
            outFile.write(&buff[0], sizeof(char), jpgLen);
        buff.clear();

        /* 
         * write zip compressed alpha channel
         */
        unsigned long len = image.cols * image.rows;
        unsigned char* uncompressedBytes = new unsigned char[len];
        unsigned char* compressedBytes = new unsigned char[std::max((int)len, 124)];
        for(int i = 3, j = 0, c = len * 4; i < c; i += 4, j++) 
            uncompressedBytes[j] = image.data[i];

        unsigned long tmplen = std::max((int) len, 128);
        int ret = 0;
        profile(ZipCompress) {
            ret = compress(compressedBytes, &tmplen, uncompressedBytes, len);
        }
        if (ret != 0) {
            outFile.close();
            log(logFATAL) << "Failed compressing alpha " << ret << " " << len;
        }
        ulong wlen = tmplen;
        outFile.write(&wlen, sizeof(wlen));
        outFile.write(compressedBytes, sizeof(char), wlen);

        /*
         * Compress audio data
         */
        len = audio.total();
        if (CCPlus::COMPRESS_AUDIO) {
            unsigned long tmp = len * 2 + 128;
            unsigned char* compressedAudio = new unsigned char[tmp];
            profile(ZipCompress) {
                ret = compress(compressedAudio, &tmp, (unsigned char*) audio.data, len * 2);
            }
            if (ret != 0) {
                outFile.close();
                log(logFATAL) << "Failed compressing audio " << ret;
            }
            // ulong is NOT unsigned long
            wlen = tmp;
            outFile.write(&wlen, sizeof(wlen));
            wlen = len * 2;
            outFile.write(&wlen, sizeof(wlen));
            outFile.write(compressedAudio, sizeof(char), tmp);
            delete[] compressedAudio;
        } else {
            wlen = 0;
            outFile.write(&wlen, sizeof(wlen));
            wlen = len * 2;
            outFile.write(&wlen, sizeof(wlen));
            outFile.write(audio.data, sizeof(int16_t), len);
        }
        outFile.close();
        delete[] uncompressedBytes;
        delete[] compressedBytes;
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

    mergeAudio(f);
}

void Frame::mergeAudio(const Frame& f) {
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
    if (input.empty()) return;
    
    if (this->getHeight() != input.rows || this->getWidth() != input.cols) {
        log(logWARN) << "image overlay expects two images with the same size"; 
        return;
    }

    if (this->getImageChannels() != 4 || input.channels() != 4) {
        log(logWARN) << "image overlay requires image to have alpha channel"; 
        return;
    }

    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            if (image.at<Vec4b>(i, j)[3] == 255) continue;
            if (image.at<Vec4b>(i, j)[3] == 0) {
                image.at<Vec4b>(i, j) = input.at<Vec4b>(i, j);
                continue;
            }
            float falpha_this = image.at<Vec4b>(i, j)[3] / 255.0;
            float falpha_img = input.at<Vec4b>(i, j)[3] / 255.0;
            float fnew_alpha = falpha_this + (1.0 - falpha_this) * falpha_img;
            for (int k = 0; k < 3; k++) {
                float x = (float) image.at<Vec4b>(i, j)[k];
                float y = (float) input.at<Vec4b>(i, j)[k];
                float ret = falpha_this * x + (1 - falpha_this) * falpha_img * y;
                ret = ret / fnew_alpha;
                image.at<Vec4b>(i, j)[k] = (uchar) std::min(255.0f, ret);
            }
            image.at<Vec4b>(i, j)[3] = (uchar) (255 * fnew_alpha);
        }
    }
}

Frame Frame::emptyFrame(int width, int height) {
    return Frame(width, height);
}

bool Frame::empty() const {
    return image.empty() && audio.empty();   
}

/**
 * Eliminate alpha, set transparent parts to black
 */
void Frame::setBlackBackground() {
    for (int i = 0; i < this->getHeight(); i++) { 
        for (int j = 0; j < this->getWidth(); j++) {
            int alpha = image.at<Vec4b>(i, j)[3];
            for(int k = 0; k < 3; k++) {
                image.at<Vec4b>(i, j)[k] = alpha * image.at<Vec4b>(i, j)[k] / 0xff;
            }
        }
    }
}

void Frame::addAlpha(const std::vector<unsigned char>& input) {
    if (this->image.empty()) return;
    if (this->image.channels() == 3)
        to4Channels();
    for (int i = 0, j = 3; i < input.size() && j < image.total() * 4; 
            i++, j += 4) {
        image.data[j] = input[i];
    }
}

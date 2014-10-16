#include "frame.hpp"
#include "profile.hpp"
#include "zip.hpp"

using namespace CCPlus;
using namespace std;

Frame::Frame() {

}

Frame::~Frame() {

}

void Frame::readZimCompressed(const cv::Mat& inData) {
    if(inData.empty()) {
        log(logERROR) << "uncompress fail due to empty input";
        return;
    }
    unsigned char* fileContent = (unsigned char*)inData.data;
    int sz = inData.total();
    unsigned char* endOfFile = fileContent + sz;
    unsigned char* ptr = fileContent;
#define NEXT(TYPE) *((TYPE*)ptr);ptr += sizeof(TYPE)

    // Image size 
    uint16_t width = NEXT(uint16_t);
    uint16_t height = NEXT(uint16_t); 

    /* 
     * Read jpg part
     */
    uint32_t jpgLen = NEXT(uint32_t);
    if (jpgLen >= 125) {
        vector<unsigned char> jpgBuff(ptr, ptr + jpgLen);
        profile(DecodeImage) {
            image = cv::imdecode(jpgBuff, CV_LOAD_IMAGE_COLOR);
        }
        mat3to4(image);
    }
    ptr += jpgLen;

    /*
     * Read alpha channel
     */
    uint32_t alphaLen = NEXT(uint32_t);
    if(alphaLen) {
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
        delete[] alphaBytes;
    }

    /*
     * Read audio channel
     */
    //uint32_t audioLen = NEXT(uint32_t);
    uint32_t audioRealByteLen = NEXT(uint32_t);
    audioRealByteLen = NEXT(uint32_t);
    if(audioRealByteLen) {
        vector<int16_t> tmp((int16_t*)ptr, (int16_t*)ptr + audioRealByteLen / 2);
        ext.audio = cv::Mat(tmp, true);
        ptr += audioRealByteLen;
    }

    if(ptr != endOfFile) {
        ext.anchorAdjustX = NEXT(int16_t);
        ext.anchorAdjustY = NEXT(int16_t);
    }
}

void Frame::frameCompress(std::function<void(void*, size_t, size_t)> write, int quality) {
    uint16_t metric; 
    // write size first
    metric = (uint16_t) image.cols;
    write(&metric, sizeof(metric), 1);
    metric = (uint16_t) image.rows;
    write(&metric, sizeof(metric), 1);

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
    uint32_t jpgLen = buff.size();
    write(&jpgLen, sizeof(jpgLen), 1);
    if (!image.empty()) {
        write(&buff[0], sizeof(char), jpgLen);
    }
    buff.clear();

    /* 
     * write zip compressed alpha channel
     */
    unsigned long len = image.cols * image.rows;
    if(len) {
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
            log(logFATAL) << "Failed compressing alpha " << ret << " " << len;
        }
        uint32_t wlen = tmplen;
        write(&wlen, sizeof(wlen), 1);
        write(compressedBytes, sizeof(char), wlen);
        delete[] uncompressedBytes;
        delete[] compressedBytes;
    } else {
        uint32_t wlen = 0;
        write(&wlen, sizeof(wlen), 1);
    }

    /*
     * Compress audio data
     */
    len = ext.audio.total();
    uint32_t wlen = 0;
    write(&wlen, sizeof(wlen), 1);
    wlen = len * 2;
    write(&wlen, sizeof(wlen), 1);
    write(ext.audio.data, sizeof(int16_t), len);
    /*
     * write anchor adjust
     */
    int16_t val = ext.anchorAdjustX;
    write(&val, sizeof(int16_t), 1);
    val = ext.anchorAdjustY;
    write(&val, sizeof(int16_t), 1);
}

cv::Mat Frame::zimCompressed(int quality) {
    std::vector<uint8_t> ret;
    ret.reserve(20);
    frameCompress([&ret](void* data, size_t sz, size_t len) {
        ret.reserve(ret.size() + sz * len);
        ret.insert(ret.end(), (uint8_t*)data, (uint8_t*)data + sz * len);
    }, quality);
    return cv::Mat(ret, true);
}

void Frame::write(const std::string& zimpath, int quality) {
    if (!stringEndsWith(zimpath, ".zim")) {
        log(logWARN) << "Zim file should use .zim ext " + zimpath;
    }
    FILE* file = fopen(zimpath.c_str(), "wb");
    if(!file) {
        log(logERROR) << "can't open file for write" << zimpath;
        return;
    }
    frameCompress([&file, &zimpath](void* _data, size_t sz, size_t len) {
        unsigned char* data = (unsigned char*)_data;
        while(len > 0) {
            size_t wlen = len;
            if(wlen > 3000)
                wlen = 3000;
            size_t wrote = fwrite(data, sz, len, file);
            if(wrote <= 0) {
                break;
            }
            len -= wrote;
            data += wrote;
        }
        if(len > 0) {
            log(logERROR) << "failed to write some data to" << zimpath;
        }
    }, quality);
    if(ftell(file) <= 0) {
        log(logWARN) << "wrote empty file:" << zimpath;
    }
    fclose(file);
}

void Frame::read(const std::string& zimpath) {
    if (!stringEndsWith(zimpath, ".zim")) {
        log(logERROR) << "Unrecgnozied file format:" + zimpath;
        return;
    }
    FILE* file = fopen(zimpath.c_str(), "rb");
    if(!file) {
        log(logERROR) << "File not exists:" << zimpath;
        return;
    }
    fseek(file, 0, SEEK_END);
    int sz = ftell(file);
    fseek(file, 0, SEEK_SET);
    if(!sz) {
        log(logWARN) << "Trying to read empty zim file:" << zimpath;
    }
    cv::Mat fileContent(1, sz, CV_8U);
    fread(fileContent.data, sizeof(unsigned char), sz, file);
    fclose(file);
    readZimCompressed(fileContent);
}

#include "frame.hpp"
#include "profile.hpp"
#include "zip.hpp"
#include "externals/lz4.h"
#include "ccplus.hpp"
#include "gpu-frame.hpp"

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
        LZ4_decompress_fast((const char*)ptr, (char*)alphaBytes, destLen);
        for(int i = 3, j = 0; j < destLen; i += 4, j++) {
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
    if(ptr != endOfFile) {
        ext.scaleAdjustX = NEXT(float);
        ext.scaleAdjustY = NEXT(float);
    }
    if (ptr != endOfFile) {
        int16_t tmp = NEXT(int16_t);
        eov = (tmp == 1 ? true : false);
    }
    if (ptr != endOfFile) {
        float bgm = NEXT(float);
        this->bgmVolume = bgm;
    }
}

void Frame::frameCompress(std::function<void(void*, size_t, size_t)> write, int quality) const {
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
            //buff = vector<unsigned char>(image.data, image.data + image.total() * 4);
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
     * Note: this is legacy feature and should be disabled
     */
    unsigned long len = image.cols * image.rows;
    if(len) {
        unsigned char* uncompressedBytes = new unsigned char[len];
        unsigned long tmplen = LZ4_compressBound(len);
        unsigned char* compressedBytes = new unsigned char[tmplen];
        for(int i = 3, j = 0, c = len * 4; i < c; i += 4, j++) 
            uncompressedBytes[j] = image.data[i];

        tmplen = LZ4_compress((const char*)uncompressedBytes, (char*)compressedBytes, len);
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
     * Write audio data
     */

    // write legacy compressed audio length
    uint32_t wlen = 0;
    write(&wlen, sizeof(wlen), 1);

    len = ext.audio.total();
    wlen = len * 2;
    write(&wlen, sizeof(wlen), 1);
    write(ext.audio.data, sizeof(int16_t), len);
    /*
     * write extra
     */
    int16_t val = ext.anchorAdjustX;
    write(&val, sizeof(int16_t), 1);
    val = ext.anchorAdjustY;
    write(&val, sizeof(int16_t), 1);
    float fval = ext.scaleAdjustX;
    write(&fval, sizeof(float), 1);
    fval = ext.scaleAdjustY;
    write(&fval, sizeof(float), 1);

    int16_t eov_val = (eov == true ? 1 : 0);
    write(&eov_val, sizeof(int16_t), 1);

    float bgm_val = bgmVolume;
    write(&bgm_val, sizeof(float), 1);
}

cv::Mat Frame::zimCompressed(int quality) const {
    std::vector<uint8_t> ret;
    ret.reserve(20);
    frameCompress([&ret](void* data, size_t sz, size_t len) {
        ret.reserve(ret.size() + sz * len);
        ret.insert(ret.end(), (uint8_t*)data, (uint8_t*)data + sz * len);
    }, quality);
    return cv::Mat(ret, true);
}

void Frame::write(const std::string& zimpath, int quality) const {
    profile(zimWrite) {
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

void Frame::toNearestPOT(int max_size, bool nearestInterpolate) {
    if(image.empty())
        return;
    int w = image.cols, h = image.rows;
    if(w > max_size) {
        w = max_size;
    }
    if(h > max_size) {
        h = max_size;
    }
    w = nearestPOT(w);
    h = nearestPOT(h);
    if(w != image.cols || h != image.rows) {
        ext.scaleAdjustX *= image.cols * 1.0f / w;
        ext.scaleAdjustY *= image.rows * 1.0f / h;
        if(nearestInterpolate)
            cv::resize(image, image, {w, h}, cv::INTER_NEAREST);
        else
            cv::resize(image, image, {w, h});
    }
}


Frame Frame::compressed(bool slower) const {
    Frame ret;
    ret.compressedFlag = true;
    ret.expectedWidth = image.cols;
    ret.expectedHeight = image.rows;
    if(slower) {
        ret.zimCompressedFlag = true;
        ret.image = zimCompressed(70);
    } else {
        ret.ext = ext;
        if(!image.empty()) {
            int total = image.total();
            int srcLen = total * 4;
            uint8_t* srcData = image.data;
            //uint8_t* srcData = new uint8_t[srcLen];
            //uint8_t* oData = image.data;
            //for(int i = 0; i < srcLen; i++) {
            //    int j = (i & 3) * total + (i >> 2);
            //    srcData[j] = oData[i];
            //}
            int sz = LZ4_compressBound(srcLen);
            char* dest = new char[sz];
            sz = LZ4_compress((char*)srcData, dest, srcLen);
            ret.image = cv::Mat(1, sz, CV_8U);
            memcpy(ret.image.data, dest, sz);
            delete[] dest;
            //delete[] srcData;
        }
    }
    return ret;
}

Frame Frame::decompressed() const {
    Frame ret;
    if(zimCompressedFlag) {
        ret.readZimCompressed(image);
    } else {
        if(!image.empty()) {
            int total = expectedWidth * expectedHeight;
            int sz = total * 4;
            ret.image = cv::Mat(expectedHeight, expectedWidth, CV_8UC4);
            //uint8_t* tmp = new uint8_t[sz];
            LZ4_decompress_fast((const char*)image.data, (char*)ret.image.data, sz);
            //uint8_t* oData = ret.image.data;
            //for(int i = 0; i < sz; i++) {
            //    int j = (i & 3) * total + (i >> 2);
            //    oData[i] = tmp[j];
            //}
            //delete[] tmp;
        }
        ret.ext = ext;
    }
    return ret;
}

bool Frame::isCompressed() const {
    return compressedFlag;
}

GPUFrame Frame::toGPU(bool premultiply) const {
    if(isCompressed()) {
        return decompressed().toGPU();
    }
    GPUFrame ret = GPUFrameCache::alloc(image.cols, image.rows);
    ret->loadFromCPU(*this);
    if(premultiply)
        ret = ret->alphaMultiplied();
    ret->ext = ext;
    return ret;
}

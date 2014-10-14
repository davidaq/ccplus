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
    }

    /*
     * Read audio channel
     */
    //uint32_t audioLen = NEXT(uint32_t);
    uint32_t audioRealByteLen = NEXT(uint32_t);
    audioRealByteLen = NEXT(uint32_t);
    if(audioRealByteLen) {
        vector<int16_t> tmp((int16_t*)ptr, (int16_t*)ptr + audioRealByteLen / 2);
        audio = cv::Mat(tmp, true);
        ptr += audioRealByteLen;
    }

    delete[] alphaBytes;
    if(ptr != endOfFile) {
        anchorAdjustX = NEXT(int16_t);
        anchorAdjustY = NEXT(int16_t);
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
    } else {
        uint32_t wlen = 0;
        write(&wlen, sizeof(wlen), 1);
    }

    /*
     * Compress audio data
     */
    len = audio.total();
    uint32_t wlen = 0;
    write(&wlen, sizeof(wlen), 1);
    wlen = len * 2;
    write(&wlen, sizeof(wlen), 1);
    write(audio.data, sizeof(int16_t), len);
    /*
     * write anchor adjust
     */
    int16_t val = anchorAdjustX;
    write(&val, sizeof(int16_t), 1);
    val = anchorAdjustY;
    write(&val, sizeof(int16_t), 1);

    delete[] uncompressedBytes;
    delete[] compressedBytes;
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
    frameCompress([file](void* data, size_t sz, size_t len) {
        fwrite(data, sz, len, file);
    }, quality);
    fclose(file);
}

void Frame::read(const std::string& zimpath) {
    if (!stringEndsWith(zimpath, ".zim")) {
        log(logERROR) << "Unrecgnozied file format: " + zimpath;
        return;
    }
    FILE* file = fopen(zimpath.c_str(), "rb");
    if(!file) {
        log(logERROR) << "File not exists: " << zimpath;
        return;
    }
    fseek(file, 0, SEEK_END);
    int sz = ftell(file);
    fseek(file, 0, SEEK_SET);
    cv::Mat fileContent(1, sz, CV_8U);
    fread(fileContent.data, sizeof(unsigned char), sz, file);
    fclose(file);
    readZimCompressed(fileContent);
}

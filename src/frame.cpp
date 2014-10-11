#include "frame.hpp"
#include "profile.hpp"
#include <cstring>

#include <vector>

using namespace CCPlus;

Frame::Frame() {

}

Frame::~Frame() {

}

void Frame::read(const std::string& zimpath) {
    if (!stringEndsWith(zimpath, ".zim")) {
        log(logERROR) << "Unrecgnozied file format: " + zimpath;
        return;
    }
    FILE* file = fopen(zimpath.c_str(), "rb");
    //File* inFile = FileManager::getInstance()->open(filepath, "rb");
    if(!file) {
        log(logERROR) << "File not exists: " << filepath;
        return;
    }
    fseek(file, 0, SEEK_END);
    int sz = inFile->getSize();
    fseek(file, 0, SEEK_SET);
    unsigned char* fileContent = new unsigned char[sz];
    unsigned char* endOfFile = fileContent + sz;
    fread(fileContent, sizeof(unsigned char), sz, file);
    fclose(file);
    //inFile->readAll(fileContent);
    //inFile->close();
    unsigned char* ptr = fileContent;
#define NEXT(TYPE) *((TYPE*)ptr);ptr += sizeof(TYPE)

    // Image size 
    ushort width = NEXT(ushort);
    ushort height = NEXT(ushort); 

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
    uint32_t audioLen = NEXT(uint32_t);
    uint32_t audioRealByteLen = NEXT(uint32_t);
    vector<int16_t> tmp((int16_t*)ptr, (int16_t*)ptr + audioRealByteLen / 2);
    audio = Mat(tmp, true);
    ptr += audioRealByteLen;

    delete[] alphaBytes;
    if(ptr != endOfFile) {
        anchorAdjustX = NEXT(int16_t);
        anchorAdjustY = NEXT(int16_t);
    }
    delete[] fileContent;
}

void Frame::write(const std::string& zimpath, int quality) {
    if (!stringEndsWith(zimpath, ".zim")) {
        log(logWARN) << "Zim file should use .zim ext " + zimpath;
    }
    typedef uint16_t ushort; 

    //FileManager* fm = FileManager::getInstance();
    //File& outFile = *fm->open(file, "wb", inMemory);
    FILE* file = fopen(zimpath.c_str(), "wb");
    ushort metric; 
    // write size first
    metric = (ushort)getWidth();
    //outFile.write(&metric, sizeof(metric));
    fwrite(&metric, sizeof(metric), 1, file);
    metric = (ushort)getHeight();
    fwrite(&metric, sizeof(metric), 1, file);
    //outFile.write(&metric, sizeof(metric));

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
    //outFile.write(&jpgLen, sizeof(jpgLen));
    fwrite(&jpgLen, sizeof(jpgLen), 1, file);
    if (!image.empty()) {
        //outFile.write(&buff[0], sizeof(char), jpgLen);
        fwrite(&buff[0], sizeof(char), jpgLen, file);
    }
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
        //outFile.close();
        fclose(file);
        log(logFATAL) << "Failed compressing alpha " << ret << " " << len;
    }
    uint32_t wlen = tmplen;
    //outFile.write(&wlen, sizeof(wlen));
    fwrite(&wlen, sizeof(wlen), 1, file);
    //outFile.write(compressedBytes, sizeof(char), wlen);
    fwrite(compressedBytes, sizeof(char), wlen, file);

    /*
     * Compress audio data
     */
    len = audio.total();
    wlen = 0;
    //outFile.write(&wlen, sizeof(wlen));
    fwrite(&wlen, sizeof(wlen), 1, file);
    wlen = len * 2;
    //outFile.write(&wlen, sizeof(wlen));
    fwrite(&wlen, sizeof(wlen), 1, file);
    //outFile.write(audio.data, sizeof(int16_t), len);
    fwrite(audio.data, sizeof(int16_t), len, file);
    /*
     * write anchor adjust
     */
    int16_t val = anchorAdjustX;
    //outFile.write(&val, sizeof(int16_t));
    fwrite(&val, sizeof(int16_t), 1, file);
    val = anchorAdjustY;
    //outFile.write(&val, sizeof(int16_t));
    fwrite(&val, sizeof(int16_t), 1, file);

    //outFile.close();
    fclose(file);
    delete[] uncompressedBytes;
    delete[] compressedBytes;
}

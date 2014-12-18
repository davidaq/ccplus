#pragma once
#include "global.hpp"

namespace CCPlus {
    struct FrameExt;
}

struct CCPlus::FrameExt {
    int anchorAdjustX = 0;
    int anchorAdjustY = 0;
    cv::Mat audio;
    float scaleAdjustX = 1;
    float scaleAdjustY = 1;
};

/* 
 * Imutable data structure
 */
class CCPlus::Frame : public CCPlus::Object {
public:
    Frame();
    ~Frame();
    cv::Mat image;
    CCPlus::FrameExt ext;
    // Mark whether this frame are the End-Of-Video
    bool eov = false;

    cv::Mat zimCompressed(int quality = 85) const;
    void readZimCompressed(const cv::Mat&);

    void write(const std::string& zim, int quality = 90) const;
    void read(const std::string& zim);

    // get a resized image that has dimensions power of 2
    void toNearestPOT(int max_size, bool nearestInterpolate=false);

    Frame compressed(bool slower=false) const;
    Frame decompressed() const;

    bool isCompressed() const;

    CCPlus::GPUFrame toGPU(bool premultiply=true) const;
private:
    void frameCompress(std::function<void(void*, size_t, size_t)>, int quality) const;
    bool compressedFlag = false;
    bool zimCompressedFlag = false;
    int expectedWidth, expectedHeight;
};

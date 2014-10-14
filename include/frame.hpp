#pragma once
#include "global.hpp"

namespace CCPlus {
    struct FrameExt;
}

struct CCPlus::FrameExt {
    int anchorAdjustX = 0;
    int anchorAdjustY = 0;
    cv::Mat audio;
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

    cv::Mat zimCompressed(int quality = 75);
    void readZimCompressed(const cv::Mat&);

    void write(const std::string& zim, int quality = 75);
    void read(const std::string& zim);
private:
    void frameCompress(std::function<void(void*, size_t, size_t)>, int quality);
};

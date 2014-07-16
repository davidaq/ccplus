#pragma once

#include <opencv2/opencv.hpp>

#include "Object.hpp"

namespace CCPlus {
    class Image;
}

class CCPlus::Image : public CCPlus::Object {
public:
    Image(const char* filepath);
    
    void write(const char* file);
    
    int getWidth() const;
    int getHeight() const;
    int getChannels() const;

    void overlayImage(const Image* img);
    
    cv::Mat* getData();
    const cv::Mat* getData() const;
    
private:
    cv::Mat data;
};

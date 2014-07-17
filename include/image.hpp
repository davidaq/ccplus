#pragma once

#include <opencv2/opencv.hpp>

#include "Object.hpp"

namespace CCPlus {
    class Image;
}

class CCPlus::Image : public CCPlus::Object {
public:
    Image(const char* filepath);
    // For testing only
    Image();
    
    void write(const char* file);
    
    int getWidth() const;
    int getHeight() const;
    int getChannels() const;

    void overlayImage(const Image* img);
    void setWhiteBackground();
    
    cv::Mat* getData();
    const cv::Mat* getData() const;

    void setData(const cv::Mat&);
    
private:
    cv::Mat data;
};

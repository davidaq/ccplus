#pragma once

#include <opencv2/opencv.hpp>

#include "Object.hpp"

namespace CCPlus {
    class Image;
}

class CCPlus::Image : public CCPlus::Object {
public:
    Image(const std::string& filepath);
    Image(int width, int height);
    Image(const cv::Mat& data);
    // For testing only
    Image();

    static Image emptyImage(int, int);
    
    void write(const std::string&);
    
    int getWidth() const;
    int getHeight() const;
    int getChannels() const;

    void overlayImage(const Image& img);
    void setWhiteBackground();
    
    cv::Mat& getData();
    const cv::Mat& getData() const;

    void setData(const cv::Mat&);
    
private:
    void rotateCWRightAngle(int angle);
    void to4Channels();

    cv::Mat data;
};

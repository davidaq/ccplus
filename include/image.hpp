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
    
    int getWidth();
    int getHeight();
    
    cv::Mat getData();
    
private:
    cv::Mat data;
    int width, height;
};

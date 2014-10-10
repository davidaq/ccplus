#pragma once
#include "global.hpp"
#include <opencv/opencv2.hpp>

class CCPlus::Frame : public CCPlus::Object {
public:
    Frame();
    ~Frame();
    cv::Mat image, audio;
    GLuint textureID = 0;
};

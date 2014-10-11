#pragma once

#include "global.hpp"

class CCPlus::GPUFrame : public Object {
public:
    void destroy();
    CCPlus::Frame toCPU();
    void load(const CCPlus::Frame& );

    int width = 0, height = 0;
    GLuint textureID = 0;
    cv::Mat audio;
};

#pragma once
#include "global.hpp"

#define GLuint unsigned int

/* 
 * Imutable data structure
 */
class CCPlus::Frame : public CCPlus::Object {
public:
    Frame();
    ~Frame();
    cv::Mat image, audio;

    int anchorAdjustX = 0;
    int anchorAdjustY = 0;

    void write(const std::string& zim);
    void read(const std::string& zim, int quality = 75);
};

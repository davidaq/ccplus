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
    GLuint textureID = 0;
    
    Frame gpu2cpu();
    Frame cpu2gpu();

    void write(const std::string& fp);
    void read(const std::string& zim);
};

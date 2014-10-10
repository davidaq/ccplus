#pragma once
#include "global.hpp"

#define GLuint unsigned int

class CCPlus::Frame : public CCPlus::Object {
public:
    Frame();
    ~Frame();
    cv::Mat image, audio;
    GLuint textureID = 0;
    
    void write(const std::string& fp);
    void read(const std::string& zim);
};

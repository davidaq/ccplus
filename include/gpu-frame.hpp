#pragma once

#include "global.hpp"

class CCPlus::GPUFrame : public Object {
public:
    ~GPUFrame();
    // @ generate a texture using the parameters
    void createTexture(int width, int height, void* pixelData=0);
    // @ must be called to ensure resource releasements
    void destroy();
    // @ reset to empty without releasing an resource
    void reset();
    // @ return a cpu copy of the frame
    CCPlus::Frame toCPU();
    // @ load a frame from cpu to gpu
    void load(const CCPlus::Frame& );
    // @ make this texture the current frame buffer, a frame buffer 
    // will be created if needed
    void bindFBO();

    int width = 0, height = 0;
    GLuint textureID = 0, fboID = 0;
    cv::Mat audio;

};

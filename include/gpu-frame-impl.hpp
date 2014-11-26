#pragma once

#include "global.hpp"
#include "frame.hpp"

class CCPlus::GPUFrameImpl : public Object {
public:
    ~GPUFrameImpl();
    // @ return a cpu copy of the frame
    CCPlus::Frame toCPU();
    // @ make this texture the current frame buffer, a frame buffer 
    void bindFBO(bool clear = true);

    int width = 0, height = 0;
    GLuint textureID = 0, fboID = 0;

    CCPlus::FrameExt ext;
    

    // @ load a frame from cpu to gpu
    // DO NOT USE THIS!
    // USE toGPU of Frame instead
    void loadFromCPU(const CCPlus::Frame& );

    // Will be called in toGPU
    GPUFrame alphaMultiplied();
};

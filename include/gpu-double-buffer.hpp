#pragma once
#include "global.hpp"

// @ Utility class to swap two GPUFrames as render buffer
class CCPlus::GPUDoubleBuffer : public CCPlus::Object {
public:
    // Create a double buffer with both frames empty
    GPUDoubleBuffer(int width, int height);
    // Create a double buffer with source set
    GPUDoubleBuffer(GPUFrame& source, int width, int height);
    // Create a double buffer with both frames set
    GPUDoubleBuffer(GPUFrame& source, GPUFrame& secondary);

    ~GPUDoubleBuffer();

    // call to make sure the final results are moved to source buffer
    // note that secondary will be destroyed once called
    void finish();

    // do opengl drawing and swap buffer, return false in callback to prevent swaping
    bool swap(const std::function<bool(CCPlus::GPUFrame&)>&);

    // get the gpu frame currently used as buffer
    GPUFrame& currentBuffer();

    // prevent the current double buffer from swaping
    static void preventSwap();
private:
    static bool prevent;
    GPUFrame* source = 0, *secondary = 0;
    bool cSource, cSecondary;

    bool finished = false;
    int currentSrc = 0;
    GPUFrame* dblBuffer[2];
};

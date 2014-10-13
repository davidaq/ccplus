#pragma once
#include "global.hpp"

class CCPlus::GPUDoubleBuffer : public CCPlus::Object {
public:
    GPUDoubleBuffer(int width, int height);
    GPUDoubleBuffer(GPUFrame& source, int width, int height);
    GPUDoubleBuffer(GPUFrame& source, GPUFrame& secondary);
    ~GPUDoubleBuffer();
    void finish();

    void swap(const std::function<void(CCPlus::GPUFrame&)>&);
    void swap(const std::function<bool(CCPlus::GPUFrame&)>&);
private:
    GPUFrame* source = 0, *secondary = 0;
    bool cSource, cSecondary;

    bool finished = false;
    int currentSrc = 0;
    GPUFrame* dblBuffer[2];
};
#pragma once

#include "renderable.hpp"

namespace CCPlus {
    class VideoRenderable;
    class VideoDecoder;
}

// @ handle videos, make'm to preprocessed image
class CCPlus::VideoRenderable : public CCPlus::Renderable {
public:
    VideoRenderable(const std::string& uri, bool audioOnly);
    ~VideoRenderable();

    //void prepare();
    void preparePart(float start, float duration);
    void releasePart(float start, float duration);
    void release();
    float getDuration();

    CCPlus::GPUFrame getGPUFrame(float time);

private:
    std::string path;
    float duration = 999999;
    bool audioOnly;
    int time2frame(float time);
    bool isUserRes = false;

    std::map<int, CCPlus::Frame> framesCache;
    std::map<int, int> frameRefer;
    std::map<int, int> frameCounter;
    CCPlus::GPUFrame lastFrame;
    int lastFrameNum = 0;
    int lastFrameImageFrameNum = -1;

    CCPlus::VideoDecoder* decoder = 0;

    float audioStartTime = -1;
    float audioEndTime = -1;
    std::vector<int16_t> audios;
};

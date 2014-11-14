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

    void prepare();
    void release();
    float getDuration();

    CCPlus::GPUFrame getGPUFrame(float time);

private:
    float duration = -1;
    void preparePart(float start, float duration);
    int time2frame(float time);

    std::string uri;
    VideoDecoder *decoder, *alpha_decoder;
    std::map<int, CCPlus::Frame> framesCache;
    std::map<int, int> frameRefer;
};

#pragma once

#include "renderable.hpp"

namespace CCPlus {
    class VideoRenderable;
    class VideoDecoder;
}

// @ handle videos, make'm to preprocessed image
class CCPlus::VideoRenderable : public CCPlus::Renderable {
public:
    VideoRenderable(const std::string& uri);

    void prepare();
    void release();
    float getDuration();

    void updateGPUFrame(GPUFrame& frame, float time);

private:
    void preparePart(float start, float duration);
    int time2frame(float time);

    std::string uri;
    VideoDecoder *decoder, *alpha_decoder;
    std::map<int, cv::Mat> framesCache;
};


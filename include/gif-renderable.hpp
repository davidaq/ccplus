#pragma once

#include "renderable.hpp"

namespace CCPlus {
    class GifRenderable;
}

// @ handle gif animations
class CCPlus::GifRenderable : public CCPlus::Renderable {
public:
    GifRenderable(const std::string& uri);

    float getDuration();
    GPUFrame getGPUFrame(float time);
    void prepare();
    void release();

private:
    std::string path;
    std::vector<std::pair<float, Frame> > framesCache;
    std::vector<GPUFrame> gpuFramesCache;
};

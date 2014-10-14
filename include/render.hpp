#pragma once
#include "global.hpp"

namespace CCPlus {
    enum BlendMode {
        NONE = -1, DEFAULT = 0,
        ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
        LIGHTEN, OVERLAY, DIFFERENCE,
        BLEND_MODE_COUNT
    };
    // Merge frames using blendmode, result will be rendered to current frame buffer
    bool mergeFrame(const CCPlus::GPUFrame& bottom,
            const CCPlus::GPUFrame& top,
            CCPlus::BlendMode blendmode);
    // Remix audio in into base
    void mergeAudio(cv::Mat& base, cv::Mat in);

    enum TrackMatteMode {
        TRKMTE_NONE = 0, TRKMTE_ALPHA, TRKMTE_ALPHA_INV, TRKMTE_LUMA, TRKMTE_LUMA_INV
    };
    // Track matte frames using mode, result will be rendered to current frame buffer
    bool trackMatte(const CCPlus::GPUFrame& color, const CCPlus::GPUFrame& alpha, CCPlus::TrackMatteMode);

    // draw a rectangle of (-1, 1, 2, 2)
    void fillSprite();

    // Draw a set of triangles. 
    // Useful when generating masks
    // size of pnts must satisfy pnts.size() % 3 = 0
    void fillTriangles(const std::vector<std::pair<float, float>>& pnts);
}

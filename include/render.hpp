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
    void mergeFrame(const CCPlus::GPUFrame& bottom,
            const CCPlus::GPUFrame& top,
            CCPlus::BlendMode blendmode);

    enum TrackMatteMode {
        TRKMTE_NONE = 0, TRKMTE_ALPHA, TRKMTE_ALPHA_INV, TRKMTE_LUMA, TRKMTE_LUMA_INV
    };
    // Track matte frames using mode, result will be rendered to current frame buffer
    void trackMatte(const CCPlus::GPUFrame& color, const CCPlus::GPUFrame& alpha, CCPlus::TrackMatteMode);

    // draw a rectangle of (-1, 1, 2, 2)
    void fillSprite();
}

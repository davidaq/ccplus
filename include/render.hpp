#pragma once
#include "global.hpp"

namespace CCPlus {
    enum BlendMode {
        NONE = -1, DEFAULT = 0,
        ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
        LIGHTEN, OVERLAY, DIFFERENCE,
        BLEND_MODE_COUNT
    };
    void mergeFrame(const CCPlus::GPUFrame& bottom,
            const CCPlus::GPUFrame& top,
            CCPlus::BlendMode blendmode);


    enum TrackMatteMode {
        TRKMTE_NONE = 0, TRKMTE_ALPHA, TRKMTE_ALPHA_INV, TRKMTE_LUMA, TRKMTE_LUMA_INV
    };
    void trackMatte(CCPlus::GPUFrame& color, CCPlus::GPUFrame& alpha, CCPlus::TrackMatteMode);

    void fillSprite();
}

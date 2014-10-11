#pragma once
#include "global.hpp"

namespace CCPlus {
    enum BlendMode {
        NONE = -1, DEFAUT = 0,
        ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
        LIGHTEN, OVERLAY, DIFFERENCE 
    };
    CCPlus::Frame mergeFrame(const CCPlus::Frame& bottom, const CCPlus::Frame& top, CCPlus::BlendMode blendmode);


    enum TrackMatteMode {
        TRKMTE_NONE = 0, TRKMTE_ALPHA, TRKMTE_ALPHA_INV, TRKMTE_LUMA, TRKMTE_LUMA_INV
    };
    CCPlus::Frame trackMatte(const CCPlus::Frame& color, const CCPlus::Frame& alpha, CCPlus::TrackMatteMode);
}

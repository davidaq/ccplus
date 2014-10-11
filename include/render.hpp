#pragma once
#include "global.hpp"

namespace CCPlus {
    enum BlendMode {
        NONE = -1, DEFAUT = 0,
        ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
        LIGHTEN, OVERLAY, DIFFERENCE 
    };
    void mergeFrame(Frame& bottom, Frame& top, BlendMode blendmode);


    enum TrackMatteMode {
        TRKMTE_NONE = 0, TRKMTE_ALPHA, TRKMTE_ALPHA_INV, TRKMTE_LUMA, TRKMTE_LUMA_INV
    };
    void trackMatte(Frame& color, Frame& alpha, TrackMatteMode);
}

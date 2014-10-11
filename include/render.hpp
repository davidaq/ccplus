#pragma once
#include "global.hpp"

namespace CCPlus {

    enum AttribLocation {
        ATTRIB_VERTEX = 1
    };

    enum BlendMode {
        NONE = -1, DEFAUT = 0,
        ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
        LIGHTEN, OVERLAY, DIFFERENCE 
    };
    void mergeFrame(CCPlus::GPUFrame& bottom, CCPlus::GPUFrame& top, CCPlus::BlendMode blendmode);


    enum TrackMatteMode {
        TRKMTE_NONE = 0, TRKMTE_ALPHA, TRKMTE_ALPHA_INV, TRKMTE_LUMA, TRKMTE_LUMA_INV
    };
    void trackMatte(CCPlus::GPUFrame& color, CCPlus::GPUFrame& alpha, CCPlus::TrackMatteMode);

    void fillSprite();
}

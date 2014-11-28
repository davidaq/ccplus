#pragma once
#include "global.hpp"

namespace CCPlus {
    // The first thing to execute right after context create 
    void initGL();

    enum BlendMode {
        NONE = -1, DEFAULT = 0,
        ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
        LIGHTEN, OVERLAY, DIFFERENCE,
        BLEND_MODE_COUNT
    };
    // Merge frames using blendmode, audio will also be mixed
    // Use `blend` instead if possible
    CCPlus::GPUFrame shaderBlend(CCPlus::GPUFrame bottom,
            CCPlus::GPUFrame top, CCPlus::BlendMode blendmode);

    enum TrackMatteMode {
        TRKMTE_NONE = 0,
        TRKMTE_ALPHA, TRKMTE_ALPHA_INV,
        TRKMTE_LUMA, TRKMTE_LUMA_INV,
        TRKMTE_MODE_COUNT
    };
    // Track matte frames using mode, audio will also be mixed
    CCPlus::GPUFrame trackMatte(CCPlus::GPUFrame color,
            CCPlus::GPUFrame alpha, CCPlus::TrackMatteMode);

    CCPlus::GPUFrame blendUsingProgram(GLuint, const GPUFrame&, const GPUFrame& top);
    
    // Use native blend if possible, fall back to shaderBlend
    // bind dst fbo before using this
    void blend(CCPlus::GPUFrame& dst, const CCPlus::GPUFrame& src, CCPlus::BlendMode blendMode);

    // draw a rectangle of (-1, 1, 2, 2)
    void fillSprite();

    // Draw a set of triangles. 
    // Useful when generating masks
    // size of pnts must satisfy pnts.size() % 3 = 0
    void fillTriangles(const std::vector<std::pair<float, float>>& pnts);
}

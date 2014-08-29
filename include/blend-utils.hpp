#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;

/*
 * Follow the SVG specification
 * http://www.w3.org/TR/SVG/filters.html#feBlendElement
 *
 * Some of them are using formulas in 
 * http://www.pegtop.net/delphi/articles/blendmodes/lighten.htm
 */

enum BlendMode {
    NONE = -1, DEFAUT = 0,
    ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
    LIGHTEN, OVERLAY, DIFFERENCE 
};

/*
 * Parameters
 * @ca premultiplied fg color 0 ~ 1
 * @cb premultiplied bg color 0 ~ 1
 * @qa fg opacity 0 ~ 1
 * @qa bg opacity 0 ~ 1
 *
 * Return
 * @ Premultiplied color 0 ~ 255
 */
// Just maybe, function pointers maybe smaller and faster than std::function
//#define BLENDER_CORE std::function<uint(uint, uint, uint, uint)>
typedef uint (*BLENDER_CORE)(uint, uint, uint, uint);
#define BLENDER std::function<cv::Vec4b(cv::Vec4b, cv::Vec4b)>
#define uint unsigned int

BLENDER_CORE getBlender(int mode);

uint defaultBlend(uint ca, uint cb, uint qa, uint qb);
uint noneBlend(uint ca, uint cb, uint qa, uint qb);
uint addBlend(uint ca, uint cb, uint qa, uint qb);
uint multiplyBlend(uint ca, uint cb, uint qa, uint qb);
uint screenBlend(uint ca, uint cb, uint qa, uint qb);
uint disolveBlend(uint ca, uint cb, uint qa, uint qb);
uint darkenBlend(uint ca, uint cb, uint qa, uint qb);
uint lightenBlend(uint ca, uint cb, uint qa, uint qb);
uint overlayBlend(uint ca, uint cb, uint qa, uint qb);
uint differenceBlend(uint ca, uint cb, uint qa, uint qb);

static inline uint alphaComposing(uint a, uint b) {
    return a + b - ((a * b) >> 8);
}
static inline Vec4b blendWithBlender(BLENDER_CORE blender, const Vec4b& top, const Vec4b& down) {
    // Some black magic constant time optimizer
    // TODO: more and a better way
    if (blender != disolveBlend && top[3] == 0) return down;
    if (blender != disolveBlend && down[3] == 0) return top;
    if ((blender == defaultBlend || blender == disolveBlend) && top[3] == 255) 
        return top; 
    if (blender == noneBlend) return top;

    Vec4b ret = {0, 0, 0, 0};

    uint topAlpha = (top[3] << 8) / 255;
    uint downAlpha = (down[3] << 8) / 255.0;
    uint retAlpha = alphaComposing(topAlpha, downAlpha);
    for (int i = 0; i < 3; i++) {
        uint ca = ((top[i] << 8) / 255 * topAlpha) >> 8;
        uint cb = ((down[i] << 8) / 255 * downAlpha) >> 8;
        ret[i] = min<uint>((blender(ca, cb, topAlpha, downAlpha) * 255) >> 8, 255);
    }
    ret[3] = (retAlpha * 255) >> 8;
    return ret;
}

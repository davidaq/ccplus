#include "blend-utils.hpp"
#include "utils.hpp"

#include "logger.hpp"

#include <map>
#include <functional>
#include <algorithm>

using namespace CCPlus;

#define uchar unsigned char

float alphaComposing(float a, float b) {
    return a + b - a * b;
}

std::map<int, BLENDER_CORE> blendMap = {
    {DEFAUT, defaultBlend},
    {NONE, noneBlend},
    {ADD, addBlend},
    {MULTIPLY, multiplyBlend},
    {SCREEN, screenBlend},
    {DISOLVE, disolveBlend},
    {DARKEN, darkenBlend},
    {LIGHTEN, lightenBlend},
    {OVERLAY, overlayBlend},
    {DIFFERENCE, differenceBlend}
};

BLENDER getBlender(int mode) {
    if (!blendMap.count(mode)) {
        log(CCPlus::logFATAL) << "Mode " << mode << " is not suppported";
    }
    BLENDER_CORE core = blendMap.at(mode); 
    return [core, mode] (Vec4b top, Vec4b down) {
        // Some black magic constant time optimizer
        // TODO: more and a better way
        if (mode != DISOLVE && top[3] == 0) return down;
        if (mode != DISOLVE && down[3] == 0) return top;
        if ((mode == DEFAUT || mode == DISOLVE) && top[3] == 255) 
            return top; 
        if (mode == NONE) return top;

        Vec4b ret = {0, 0, 0, 0};
        float topAlpha = top[3] / 255.0;
        float downAlpha = down[3] / 255.0;
        float retAlpha = alphaComposing(topAlpha, downAlpha);
        for (int i = 0; i < 3; i++) {
            //if (mode == MULTIPLY) {
            //    float ca = top[i] / 255.0;
            //    float cb = down[i] / 255.0;
            //    ret[i] = between<int>(ca * cb * 255.0, 0, 255);
            //    continue;
            //}
            float ca = top[i] / 255.0 * topAlpha;
            float cb = down[i] / 255.0 * downAlpha;
            ret[i] = between<int>(core(ca, cb, topAlpha, downAlpha) / retAlpha * 255.0, 0, 255);
        }
        ret[3] = retAlpha * 255.0;
        return ret;
    };
}

float defaultBlend(float ca, float cb, float qa, float qb) {
    return (1 - qa) * cb + ca;
}

float noneBlend(float ca, float cb, float qa, float qb) {
    return ca;
}

float addBlend(float ca, float cb, float qa, float qb) {
    return ca + cb;
}

float multiplyBlend(float ca, float cb, float qa, float qb) {
    return (1 - qa) * cb + (1 - qb) * ca + ca * cb;
}

float screenBlend(float ca, float cb, float qa, float qb) {
    return cb + ca - ca * cb;
}

float disolveBlend(float ca, float cb, float qa, float qb) {
    int range = 100.0 * qa;
    int v = std::rand() % 100;
    if (v < range) return ca;
    return cb;
}

float darkenBlend(float ca, float cb, float qa, float qb) {
    return std::min<float>((1 - qa) * cb + ca,
            (1 - qb) * ca + cb);
}

float lightenBlend(float ca, float cb, float qa, float qb) {
    return std::max<float>((1 - qa) * cb + ca,
            (1 - qb) * ca + cb);
}

// TODO: Somebody comes to verify it please. 
// It's slightly different from the formula in Wikipedia
float overlayBlend(float ca, float cb, float qa, float qb) {
    if (ca < 0.5)
        return multiplyBlend(ca, cb, qa, qb) * 2.0; 
    return 1 - 2 * (1 - ca) * (1 - cb);
}

float differenceBlend(float ca, float cb, float qa, float qb) {
    return std::abs<float>(ca - cb);
}

#include "blend-utils.hpp"
#include "utils.hpp"

#include "logger.hpp"

#include <map>
#include <functional>
#include <algorithm>

using namespace CCPlus;

#define uchar unsigned char

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

BLENDER_CORE getBlender(int mode) {
    if (!blendMap.count(mode)) {
        log(CCPlus::logFATAL) << "Mode " << mode << " is not suppported";
        mode = NONE;
    }
    return blendMap[mode];
}

uint defaultBlend(uint ca, uint cb, uint qa, uint qb) {
    return (((0x100 - qa) * cb) >> 8) + ca;
}

uint noneBlend(uint ca, uint cb, uint qa, uint qb) {
    return ca;
}

uint addBlend(uint ca, uint cb, uint qa, uint qb) {
    return ca + cb;
}

uint multiplyBlend(uint ca, uint cb, uint qa, uint qb) {
    return (((0x100 - qa) * cb) >> 8) + (((0x100 - qb) * ca) >> 8) + ((ca * cb) >> 8);
}

uint screenBlend(uint ca, uint cb, uint qa, uint qb) {
    return cb + ca - ((ca * cb) >> 8);
}

uint disolveBlend(uint ca, uint cb, uint qa, uint qb) {
    int v = std::rand() & 0xff;
    return v < qa ? ca:cb;
}

uint darkenBlend(uint ca, uint cb, uint qa, uint qb) {
    return std::min<uint>((((0x100 - qa) * cb) >> 8) + ca,
            (((0x100 - qb) * ca) >> 8) + cb);
}

uint lightenBlend(uint ca, uint cb, uint qa, uint qb) {
    return std::max<uint>((((0x100 - qa) * cb) >> 8) + ca,
            (((0x100 - qb) * ca) >> 8) + cb);
}

uint overlayBlend(uint ca, uint cb, uint qa, uint qb) {
    if (cb < 128)
        return multiplyBlend(ca, cb, qa, qb) * 2; 
    return 0x100 - min<uint>(((0x100 - ca) * (0x100 - cb) >> 8) * 2, 0x100);
}

uint differenceBlend(uint ca, uint cb, uint qa, uint qb) {
    return ca > cb ? ca - cb : cb - ca;
}

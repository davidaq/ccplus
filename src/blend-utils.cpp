#include "blend-utils.hpp"

#include "logger.hpp"

#include <map>
#include <functional>
#include <algorithm>

#define uchar unsigned char

std::map<int, BLENDER> blendMap = {
    {DEFAUT, defaultBlend},
    {NONE, noneBlend},
    {ADD, addBlend},
    {MULTIPLY, multiplyBlend},
    {SCREEN, screenBlend}
};

const BLENDER& getBlender(int mode) {
    if (!blendMap.count(mode)) {
        log(CCPlus::logFATAL) << "Mode " << mode << " is not suppported";
        return blendMap[DEFAUT];
    }
    return blendMap.at(mode);
}

Vec4b defaultBlend(Vec4b top, Vec4b down) {
    if (top[3] == 255) return top;
    if (top[3] == 0) return down;
    float falpha_this = top[3] / 255.0;
    float falpha_img = down[3] / 255.0;
    float fnew_alpha = falpha_this + (1.0 - falpha_this) * falpha_img;
    Vec4b retColor = {0, 0, 0, 0};
    for (int k = 0; k < 3; k++) {
        float x = (float) top[k];
        float y = (float) down[k];
        float ret = falpha_this * x + (1 - falpha_this) * falpha_img * y;
        ret = ret / fnew_alpha;
        retColor[k] = (uchar) std::min(255.0f, ret);
    }
    retColor[3] = (uchar) (255 * fnew_alpha);
    return retColor;
}

Vec4b noneBlend(Vec4b top, Vec4b down) {
    return top;
}

Vec4b addBlend(Vec4b top, Vec4b down) {
    for (int i = 0; i < 3; i++) {
        float alpha = top[3] / 255.0;
        top[i] = (uchar)std::min<int>(255, (int) top[i] * alpha + down[i]);
    }
    top[3] = down[3];
    return top;
}

Vec4b multiplyBlend(Vec4b top, Vec4b down) {
    for (int i = 0; i < 3; i++) {
        // It seems better
        float alpha = top[3] / 255.0;
        // Don't panic. It's Black Magic ! 
        float c = top[i] / 255.0;
        c = (1.0 - c) * (1 - alpha) + c;
        top[i] = (uchar)std::min<int>(255, 
                (int) 255.0 * (c * down[i] / 255.0));
    }
    top[3] = down[3];
    return top;
}

Vec4b screenBlend(Vec4b top, Vec4b down) {
    for (int i = 0; i < 3; i++) {
        float alpha = top[3] / 255.0f;
        float x = top[i] / 255.0f * alpha;
        float y = down[i] / 255.0f;
        float ret = 1.0f - (1.0f - x) * (1.0f - y);
        top[i] = (uchar) std::min<int>(255, ret * 255.0f);
    }
    top[3] = down[3];
    return top;
}

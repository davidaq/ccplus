#include "blend-utils.hpp"

#include "logger.hpp"

#include <map>
#include <functional>

std::map<int, BLENDER> blendMap = {
    {DEFAUT, defaultBlend}
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

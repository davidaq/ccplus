#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(mask) {
    if (parameters.size() < 2) {
        log(logERROR) << "Not enough parameters for mask";
        return frame;
    }
    std::vector<std::pair<float, float>> pnts;
    int kwidth = parameters[1] / frame->ext.scaleAdjustX;
    int kheight = parameters[0] / frame->ext.scaleAdjustY;
    int ksize = (kwidth + kheight) / 2;
    int sz = parameters.size() / 2 - 1;

    for (int i = 1; i <= sz; i++) {
        if (i > 1 && 
            parameters[i * 2] == parameters[i * 2 - 2] && 
            parameters[i * 2 + 1] == parameters[i * 2 - 1])
            continue;
        pnts.push_back({
                parameters[i * 2 + 1] / frame->ext.scaleAdjustX / width * 2.0 - 1.0, 
                parameters[i * 2] / frame->ext.scaleAdjustY / height * 2.0 - 1.0});
    }
    pnts = CCPlus::triangulate(pnts);

    GLProgramManager* manager = GLProgramManager::getManager();

    if (ksize == 0) {
        GLuint program = manager->getProgram(filter_mask);
        glUseProgram(program);
        GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
        ret->ext = frame->ext;
        ret->bindFBO();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame->textureID);

        if (pnts.size() == 0)
            return ret;
        fillTriangles(pnts);
        return ret;
    } 
    GLuint program = manager->getProgram(filter_mask_gen);
    glUseProgram(program);

    GPUFrame mask = GPUFrameCache::alloc(frame->width, frame->height);
    mask->bindFBO();
    fillTriangles(pnts);

    //mask = Filter("gaussian").apply(mask, {(float)ksize, 1}, mask->width, mask->height);
    if (kwidth == kheight) {
        mask = Filter("gaussian").apply(mask, {(float)kwidth, 1}, mask->width, mask->height);
    } else {
        mask = Filter("gaussian").apply(mask, {(float)kwidth, 3}, mask->width, mask->height);
        mask = Filter("gaussian").apply(mask, {(float)kheight, 2}, mask->width, mask->height);
    }

    program = manager->getProgram(filter_mask_merge);
    return blendUsingProgram(program, frame, mask);
}

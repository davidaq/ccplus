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
    int kwidth = parameters[1];
    int kheight = parameters[0];
    int ksize = (kwidth + kheight) / 2;
    int sz = parameters.size() / 2 - 1;

    for (int i = 1; i <= sz; i++) {
        if (i > 1 && 
            parameters[i * 2] == parameters[i * 2 - 2] && 
            parameters[i * 2 + 1] == parameters[i * 2 - 1])
            continue;
        pnts.push_back({
                parameters[i * 2 + 1] / width * 2.0 - 1.0, 
                parameters[i * 2] / height * 2.0 - 1.0});
    }
    pnts = CCPlus::triangulate(pnts);

    GLProgramManager* manager = GLProgramManager::getManager();

    if (ksize == 0) {
        GLuint program = manager->getProgram(
                "filter_mask",
                "shaders/fill.v.glsl",
                "shaders/filters/mask.f.glsl"
                );
        glUseProgram(program);
        GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
        ret->ext = frame->ext;
        ret->bindFBO();

        glUniform1i(glGetUniformLocation(program, "tex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame->textureID);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (pnts.size() == 0)
            return ret;
        fillTriangles(pnts);
        return ret;
    } 
    GLuint program = manager->getProgram(
            "filter_mask_gen",
            "shaders/fill.v.glsl",
            "shaders/filters/mask_gen.f.glsl");
    glUseProgram(program);

    GPUFrame mask = GPUFrameCache::alloc(frame->width, frame->height);
    mask->bindFBO();
    fillTriangles(pnts);

    //mask = Filter("gaussian").apply(mask, {(float)ksize, 1}, mask->width, mask->height);
    mask = Filter("gaussian").apply(mask, {(float)kwidth, 3}, mask->width, mask->height);
    mask = Filter("gaussian").apply(mask, {(float)kheight, 2}, mask->width, mask->height);

    program = manager->getProgram(
            "filter_mask_merge",
            "shaders/fill.v.glsl",
            "shaders/filters/mask_merge.f.glsl");
    return blendUsingProgram(program, frame, mask);
}

#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(hsl) {
    if (parameters.size() < 3) {
        log(logERROR) << "Insufficient parameters for hsl effect";
        return frame;
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint hue, sat, lit;
    GLuint program = manager->getProgram(filter_hsl, &hue, &sat, &lit);
    glUseProgram(program);

    GPUFrame ret = GPUFrameCache::alloc(width, height);
    ret->bindFBO(false);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);

    glUniform1f(hue, parameters[0] / 180.0);
    glUniform1f(sat, parameters[1]);
    glUniform1f(lit, parameters[2]);

    fillSprite();
    ret->ext = frame->ext;
    return ret;
}

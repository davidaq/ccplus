#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"
#include "render.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(opacity) {
    if (parameters.size() == 0) {
        log(logERROR) << "Not enough parameters for opacity";
        return frame;
    }

    float opa = parameters[0];
    if (std::abs(opa - 1.0f) < 0.001) {
        return frame;
    }

    GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
    ret->ext = frame->ext;
    ret->bindFBO(false);

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
        "filter_opacity",
        "shaders/fill.v.glsl",
        "shaders/filters/opacity.f.glsl"
    );
    glUseProgram(program);

    GLuint location = glGetUniformLocation(program, "opa");
    glUniform1f(location, opa);

    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);

    fillSprite();
    return ret;
}

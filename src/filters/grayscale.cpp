#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"

using namespace CCPlus;

CCPLUS_FILTER(grayscale) {
    if (parameters.size() < 8) {
        log(logERROR) << "Insufficient parameters for grayscale effect";
        return frame;
    }
    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_grayscale",
            "shaders/fill.v.glsl",
            "shaders/filters/grayscale.f.glsl"
            );
    glUseProgram(program);

    GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
    ret->bindFBO(false);

    static float weights[6];
    for (int i = 0; i < 6; i++) 
        weights[i] = parameters[i] / 100.0;

    glUniform1fv(glGetUniformLocation(program, "weights"), 6, weights);
    glUniform2f(glGetUniformLocation(program, "hue_sat"), 
            parameters[6] / 360.0f, parameters[7]);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);

    fillSprite();

    return ret;
}

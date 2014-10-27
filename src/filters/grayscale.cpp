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
    GLuint weightsU, hue_satU;
    GLuint program = manager->getProgram(filter_grayscale, &weightsU, &hue_satU);
    glUseProgram(program);

    GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
    ret->bindFBO(false);

    static float weights[6];
    for (int i = 0; i < 6; i++) 
        weights[i] = parameters[i] / 100.0;

    glUniform1fv(weightsU, 6, weights);
    glUniform2f(hue_satU, parameters[6] / 360.0f, parameters[7]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);

    fillSprite();

    return ret;
}

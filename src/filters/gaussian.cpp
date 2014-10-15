#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(gaussian) {
    if (parameters.size() < 2) {
        log(logERROR) << "In sufficient parameters for gaussian filter";
        return frame;
    }
    int size = (int) parameters[0];
    // Tmp
    size = 35;
    int direction = (int) parameters[1];

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_gaussian",
            "shaders/filters/gaussian.v.glsl",
            "shaders/filters/gaussian.f.glsl"
            );
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    // Divide by 4 for acceleration
    int ksize = round(size / 4);
    glUniform1i(glGetUniformLocation(program, "ksize"), ksize);

    GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
    GPUFrame tmp = GPUFrameCache::alloc(frame->width, frame->height);
    if (direction != 2) { // With X
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame->textureID);
        glUniform2f(glGetUniformLocation(program, "pixelOffset"), 1.0f / frame->width, 0);
        tmp->bindFBO();
        glClearColor(0, 0, 0, 0);
        fillSprite(); // Draw on tmp
    } 
    if (direction != 3) { // With Y
        glUniform2f(glGetUniformLocation(program, "pixelOffset"), 0, 1.0f / frame->width);
        if (direction == 2) { // never go x
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, frame->textureID);
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tmp->textureID);
        }
        ret->bindFBO();
        glClearColor(0, 0, 0, 0);
        fillSprite();
    } else {
        tmp->ext = frame->ext;
        return tmp;
    }
    ret->ext = frame->ext;
    return ret;
}

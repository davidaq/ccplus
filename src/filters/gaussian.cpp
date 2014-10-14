#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "externals/triangulate.h"
#include "gpu-double-buffer.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(gaussian) {
    if (parameters.size() < 2) {
        log(logERROR) << "In sufficient parameters for gaussian filter";
        return;
    }
    int size = (int) parameters[0];
    int direction = (int) parameters[1];

    //TODO
    GPUDoubleBuffer dbuffer(buffer, frame.width, frame.height);

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_gaussian",
            "shaders/fill.v.glsl",
            "shaders/filters/gaussian.f.glsl"
            );
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame.textureID);
                
    // Divide by 4 for acceleration
    int ksize = round(size / 4);
    glUniform1i(glGetUniformLocation(program, "ksize"), ksize);

    if (direction != 2) { // With X
        dbuffer.swap([&frame, &program] (GPUFrame& src) {
            glUniform2f(glGetUniformLocation(program, "pixelOffset"), 1.0f / frame.width, 0);
            glClearColor(0, 0, 0, 0);
            fillSprite();
            return true;
        });
    } 
    if (direction != 3) { // With Y
        dbuffer.swap([&frame, &program] (GPUFrame& src) {
            glUniform2f(glGetUniformLocation(program, "pixelOffset"), 0, 1.0f / frame.width);
            glClearColor(0, 0, 0, 0);
            fillSprite();
            return true;
        });
    }
    dbuffer.finish();
}

#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "externals/triangulate.h"
#include "gpu-double-buffer.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(hsl) {
    if (parameters.size() < 3) {
        log(logERROR) << "Insufficient parameters for hsl effect";
        return;
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_hsl",
            "shaders/fill.v.glsl",
            "shaders/filters/hsl.f.glsl");
    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame.textureID);

    glUniform1f(glGetUniformLocation(program, "hue"), parameters[0] / 180.0);
    glUniform1f(glGetUniformLocation(program, "sat"), parameters[1]);
    glUniform1f(glGetUniformLocation(program, "lit"), parameters[2]);

    fillSprite();
}

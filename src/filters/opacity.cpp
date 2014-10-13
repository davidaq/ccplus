#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(opacity) {
    if (parameters.size() == 0)
        return;

    float opa = parameters[0];

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
    glBindTexture(GL_TEXTURE_2D, frame.textureID);

    fillSprite();
}

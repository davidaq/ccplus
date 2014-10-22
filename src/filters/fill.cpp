#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"

using namespace CCPlus;

CCPLUS_FILTER(fill) {
    if (parameters.size() < 3) {
        log(logERROR) << "Insufficient parameters for fill effect";
        return frame;
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_fill",
            "shaders/fill.v.glsl",
            "shaders/filters/fill.f.glsl"
            );
    glUseProgram(program);

    frame->bindFBO(false);

    glUniform3f(glGetUniformLocation(program, "col"), 
            parameters[0] / 255.0f, 
            parameters[1] / 255.0f, 
            parameters[2] / 255.0f);

    fillSprite();

    return frame;
}

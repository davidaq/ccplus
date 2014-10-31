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
    GLuint col;
    GLuint program = manager->getProgram(filter_fill, &col);
    glUseProgram(program);

    frame->bindFBO(false);

    glUniform3f(col,
            parameters[0] / 255.0f, 
            parameters[1] / 255.0f, 
            parameters[2] / 255.0f);

    fillSprite();

    return frame;
}

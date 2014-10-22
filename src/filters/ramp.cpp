#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(ramp) {
    if (parameters.size() < 12) {
        log(logERROR) << "Insufficient parameters for ramp effect";
        return frame;
    }
    float alpha = 1 - parameters[11];
    int type = parameters[0];
    float start_x = parameters[1] / frame->width;
    float start_y = parameters[2] / frame->height;
    float sr = parameters[3] / 255.0;
    float sg = parameters[4] / 255.0;
    float sb = parameters[5] / 255.0;

    float end_x = parameters[6] / frame->width;
    float end_y = parameters[7] / frame->height;
    float er = parameters[8] / 255.0;
    float eg = parameters[9] / 255.0;
    float eb = parameters[10] / 255.0;

    std::string fshader = "";
    std::string name = "";
    if (type < 0) {
        name = "filter_ramp_linear";
        fshader = "shaders/filters/ramp_linear.f.glsl";
    } else {
        name = "filter_ramp_radial";
        fshader = "shaders/filters/ramp_radial.f.glsl";
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            name,
            "shaders/fill.v.glsl",
            fshader
            );
    glUseProgram(program);

    GPUFrame tmp_frame = GPUFrameCache::alloc(frame->width, frame->height);
    tmp_frame->bindFBO(false);

    float dx = start_x - end_x;
    float dy = start_y - end_y;
    float dis = sqrt(dx * dx + dy * dy);
    glUniform1f(glGetUniformLocation(program, "alpha"), alpha);
    glUniform1f(glGetUniformLocation(program, "dis"), dis);
    glUniform2f(glGetUniformLocation(program, "start"), start_x, start_y);
    glUniform2f(glGetUniformLocation(program, "end"), end_x, end_y);
    glUniform3f(glGetUniformLocation(program, "s_rgb"), sr, sg, sb);
    glUniform3f(glGetUniformLocation(program, "e_rgb"), er, eg, eb);

    fillSprite();

    return mergeFrame(frame, tmp_frame, (BlendMode)0);
}

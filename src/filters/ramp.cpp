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
    GLProgram programName = (type < 0) ? filter_ramp_linear : filter_ramp_radial;
    if (type < 0) {
        name = "filter_ramp_linear";
        fshader = "shaders/filters/ramp_linear.f.glsl";
    } else {
        name = "filter_ramp_radial";
        fshader = "shaders/filters/ramp_radial.f.glsl";
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint alphaU, disU, startU, endU, s_rgbU, e_rgbU;
    GLuint program = manager->getProgram(programName, &alphaU, &disU, &startU, &endU, &s_rgbU, &e_rgbU);
    glUseProgram(program);

    GPUFrame tmp_frame = GPUFrameCache::alloc(frame->width, frame->height);
    tmp_frame->bindFBO(false);

    float dx = start_x - end_x;
    float dy = start_y - end_y;
    float dis = sqrt(dx * dx + dy * dy);
    glUniform1f(alphaU, alpha);
    glUniform1f(disU, dis);
    glUniform2f(startU, start_x, start_y);
    glUniform2f(endU, end_x, end_y);
    glUniform3f(s_rgbU, sr, sg, sb);
    glUniform3f(e_rgbU, er, eg, eb);

    fillSprite();

    return mergeFrame(frame, tmp_frame, (BlendMode)0);
}

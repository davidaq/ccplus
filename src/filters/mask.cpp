#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(mask) {
    if (parameters.size() <= 0) {
        log(logERROR) << "Not enough parameters for mask";
        return;
    }
    std::vector<std::pair<float, float>> pnts;
    int kwidth = parameters[0];
    int kheight = parameters[1];
    int sz = parameters.size() / 2 - 1;

    for (int i = 1; i <= sz; i++) {
        pnts.push_back({
                parameters[i * 2] / width * 2.0 - 1.0, 
                parameters[i * 2 + 1] / height * 2.0 - 1.0});
    }
    pnts = CCPlus::triangulate(pnts);

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
        "filter_mask",
        "shaders/fill.v.glsl",
        "shaders/filters/mask.f.glsl"
    );
    glUseProgram(program);
    
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame.textureID);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    fillTriangles(pnts);
}

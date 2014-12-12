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
    glClearColor(
        parameters[0] / 255.0f, 
        parameters[1] / 255.0f, 
        parameters[2] / 255.0f,
        1.0f);
    frame->bindFBO(false);
    glClearColor(0, 0, 0, 0);

    return frame;
}

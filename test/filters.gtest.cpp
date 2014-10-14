#include "gtest/gtest.h"

#include "frame.hpp"
#include "gpu-frame.hpp"
#include "logger.hpp"
#include "glprogram-manager.hpp"

#include "context.hpp"
#include "platform.hpp"
#include "filter.hpp"
#include "profile.hpp"

using namespace std;
using namespace CCPlus;

TEST(Filter, Mask) {
    createGLContext();

    Frame tmp;
    tmp.image = cv::imread("test/res/test2.jpg");
    mat3to4(tmp.image);
    GPUFrame src;
    src.load(tmp);

    GPUFrame dst;
    dst.createTexture(src.width, src.height);
    dst.bindFBO();
    glClear(GL_COLOR_BUFFER_BIT);
    // TEST
    
    Filter* filter = new Filter("mask");
    //filter->apply(src, {0, 0, 100, 100, 200, 100, 200, 200, 100, 200}, 640, 852);
    filter->apply(dst, src, {0, 0, 100, 100, 125, 100, 150, 50, 175, 100, 200, 100, 150, 300}, 640, 852);

    imwrite("tmp/ret.png", dst.toCPU().image);
}

TEST(Filter, Gaussian) {
    createGLContext();

    Frame tmp;
    tmp.image = cv::imread("test/res/test2.jpg");
    mat3to4(tmp.image);
    GPUFrame src;
    src.load(tmp);

    GPUFrame dst;
    dst.createTexture(src.width, src.height);
    dst.bindFBO();
    glClear(GL_COLOR_BUFFER_BIT);

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_gaussian",
            "shaders/fill.v.glsl",
            "shaders/filters/gaussian.f.glsl"
            );
    
    profileBegin(Gaussian);
    Filter* filter = new Filter("gaussian");
    filter->apply(dst, src, {35, 1}, 640, 852);
    glFinish();
    profileEnd(Gaussian);
    Profiler::flush();

    imwrite("tmp/ret.png", dst.toCPU().image);
}

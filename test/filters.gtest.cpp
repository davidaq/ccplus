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

TEST(Filter, Transform) {
    createGLContext();

    Frame tmp;
    tmp.image = cv::imread("test/res/test1.jpg");
    mat3to4(tmp.image);
    GPUFrame src;
    src.load(tmp);

    GPUFrame dst;
    dst.createTexture(500, 500);
    dst.bindFBO();
    glClear(GL_COLOR_BUFFER_BIT);
    // TEST
    
    Filter* filter = new Filter("transform");
    filter->apply(dst, src, {250, 280, 0, 0, 0, 0, 1, 1, 1, 0, 0, 90}, 500, 500);

    imwrite("tmp/ret.png", dst.toCPU().image);
}

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

    //GLProgramManager* manager = GLProgramManager::getManager();
    //GLuint program = manager->getProgram(
    //        "filter_gaussian",
    //        "shaders/fill.v.glsl",
    //        "shaders/filters/gaussian.f.glsl"
    //        );
    
    profileBegin(Gaussian);
    Filter* filter = new Filter("gaussian");
    filter->apply(dst, src, {35, 1}, 640, 852);
    glFinish();
    profileEnd(Gaussian);
    Profiler::flush();

    imwrite("tmp/ret.png", dst.toCPU().image);
}

TEST(Filter, 4Color) {
    createGLContext();

    Frame tmp;
    tmp.image = cv::imread("test/res/test1.jpg");
    mat3to4(tmp.image);
    GPUFrame src;
    src.load(tmp);

    GPUFrame dst;
    dst.createTexture(src.width, src.height);
    dst.bindFBO();
    glClear(GL_COLOR_BUFFER_BIT);

    profileBegin(4color);
    Filter* filter = new Filter("4color");
    filter->apply(dst, src, {
            0, 0, 255, 0, 0, 
            279, 0, 0, 255, 0, 
            0, 242, 0, 0, 255, 
            140, 121, 255, 255, 255, 
            5, 0.5, 0}, 280, 243);
    glFinish();
    profileEnd(4color);
    Profiler::flush();

    imwrite("tmp/ret.png", dst.toCPU().image);
}

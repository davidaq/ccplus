#include "gtest/gtest.h"

#include "frame.hpp"
#include "logger.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"

#include "context.hpp"
#include "platform.hpp"
#include "filter.hpp"
#include "profile.hpp"

using namespace std;
using namespace CCPlus;

static inline Frame testFilter(const std::string srcFile, const std::string filterName, const std::vector<float>& parameters, 
        int width=-1, int height=-1) {
    static bool needContext = true;
    if(needContext) {
        createGLContext();
        needContext = false;
    }

    Frame tmp;
    tmp.image = cv::imread(srcFile);
    mat3to4(tmp.image);
    GPUFrame src = GPUFrameCache::alloc(tmp.image.cols, tmp.image.rows);
    src->load(tmp);

    if(width < 0)
        width = tmp.image.cols;
    if(height < 0)
        height = tmp.image.rows;
    return Filter(filterName).apply(src, parameters, width, height)->toCPU();
}

TEST(Filter, Transform) {
    imwrite("tmp/transform.png", testFilter("test/res/test1.jpg", "transform",
                {250, 280, 0, 0, 0, 0, 1, 1, 1, 0, 0, 90}, 500, 500).image);
}

TEST(Filter, Mask) {
    imwrite("tmp/mask.jpg", testFilter("test/res/test2.jpg", "mask",
                {33, 33, 300, 100, 325, 100, 350, 50, 375, 100, 400, 100, 350, 300}, 640, 852).image);
}

TEST(Filter, Gaussian) {
    imwrite("tmp/gaussian.png", testFilter("test/res/test2.jpg", "gaussian",
                {31, 1}, 640, 852).image);
}

TEST(Filter, 4Color) {
    imwrite("tmp/4color.png", testFilter("test/res/test1.jpg", "4color",
                { 0, 0, 255, 0, 0, 
                279, 0, 0, 255, 0, 
                0, 242, 0, 0, 255, 
                140, 121, 255, 255, 255, 
                5, 0.5, 0}, 280, 243).image);
}

TEST(Filter, RampLinear) {
    imwrite("tmp/ramp-linear.png", testFilter("test/res/test2.jpg", "ramp",
                {-1, 300, 0, 0, 0, 0, 300, 800, 255, 255, 255, 0.2}, 640, 852).image);
}

TEST(Filter, RampRadial) {
    imwrite("tmp/ramp-radial.png", testFilter("test/res/test2.jpg", "ramp",
                {1, 300, 0, 0, 0, 0, 300, 800, 255, 255, 255, 0.2}, 640, 852).image);
}

TEST(Filter, HSL) {
    imwrite("tmp/hsl.png", testFilter("test/res/test2.jpg", "hsl",
                {135, 1.1, 1.5}, 640, 852).image);
}

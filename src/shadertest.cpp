/**************************************************************************
 * The following are test utilities which are not parts of the ccplus api.
 * This used to examine problems when porting to a target platform.
 * Just call CCPLUS_TEST(); in a C environment to run the tests
 **************************************************************************/
#include "global.hpp"
#include "filter.hpp"
#include "frame.hpp"
#include "gpu-frame.hpp"
#include "platform.hpp"
#include "render.hpp"
#include "parallel-executor.hpp"

using namespace CCPlus;
std::string opos;

GPUFrame imreadAsset(const std::string& path) {
    cv::Mat mat = readAsset(path.c_str());
    mat = cv::imdecode(mat, CV_LOAD_IMAGE_UNCHANGED);
    mat3to4(mat);
    Frame frame;
    frame.image = mat;
    return frame.toGPU();
}

static inline Frame testFilter(const std::string srcFile, const std::string filterName, const std::vector<float>& parameters, 
        int width=-1, int height=-1) {
    GPUFrame src = imreadAsset(srcFile);
    if(width < 0)
        width = src->width;
    if(height < 0)
        height = src->height;
    return Filter(filterName).apply(src, parameters, width, height)->toCPU();
}

static inline std::string P(const std::string& p) {
    return opos + "/" + p;
}

static inline Frame testMerge(GPUFrame a, GPUFrame b, BlendMode mode) {
    return mergeFrame(a, b, mode)->toCPU();
}

static inline Frame testTrkMat(GPUFrame a, GPUFrame b, TrackMatteMode mode) {
    return trackMatte(a, b, mode)->toCPU();
}

void testMe() {
    imwrite(P("load.png"), imreadAsset("test/res/test1.jpg")->toCPU().image);

    imwrite(P("transform.png"), testFilter("test/res/test1.jpg", "transform",
                {250, 280, 0, 0, 0, 0, 1, 1, 1, 0, 0, 90}, 500, 500).image);
    imwrite(P("mask.png"), testFilter("test/res/test2.jpg", "mask",
                {50, 50, 300, 100, 325, 100, 350, 50, 375, 100, 400, 100, 350, 300}, 640, 852).image);
    imwrite(P("gaussian.png"), testFilter("test/res/test2.jpg", "gaussian",
                {47, 1}, 640, 852).image);
    imwrite(P("4color.png"), testFilter("test/res/test1.jpg", "4color",
                { 0, 0, 255, 0, 0, 
                279, 0, 0, 255, 0, 
                0, 242, 0, 0, 255, 
                140, 121, 255, 255, 255, 
                5, 0.5, 0}, 280, 243).image);
    imwrite(P("ramp-linear.png"), testFilter("test/res/test2.jpg", "ramp",
                {-1, 300, 0, 0, 0, 0, 300, 800, 255, 255, 255, 0.2}, 640, 852).image);
    imwrite(P("ramp-radial.png"), testFilter("test/res/test2.jpg", "ramp",
                {1, 300, 0, 0, 0, 0, 300, 800, 255, 255, 255, 0.2}, 640, 852).image);
    imwrite(P("hsl.png"), testFilter("test/res/test2.jpg", "hsl",
                {135, 1.1, 1.5}, 640, 852).image);
    // test merge
    GPUFrame a = imreadAsset("test/1.png");
    GPUFrame b = imreadAsset("test/2.png");
    
    imwrite(P("b-none.png"), testMerge(a, b, NONE).image);
    imwrite(P("b-default.png"), testMerge(a, b, DEFAULT).image);
    imwrite(P("b-add.png"), testMerge(a, b, ADD).image);
    imwrite(P("b-multiply.png"), testMerge(a, b, MULTIPLY).image);
    imwrite(P("b-screen.png"), testMerge(a, b, SCREEN).image);
    imwrite(P("b-disolve.png"), testMerge(a, b, DISOLVE).image);
    imwrite(P("b-darken.png"), testMerge(a, b, DARKEN).image);
    imwrite(P("b-lighten.png"), testMerge(a, b, LIGHTEN).image);
    imwrite(P("b-overlay.png"), testMerge(a, b, OVERLAY).image);
    imwrite(P("b-difference.png"), testMerge(a, b, DIFFERENCE).image);

    imwrite(P("t-alpha.png"), testTrkMat(a, b, TRKMTE_ALPHA).image);
    imwrite(P("t-alpha_inv.png"), testTrkMat(a, b, TRKMTE_ALPHA_INV).image);
    imwrite(P("t-luma.png"), testTrkMat(a, b, TRKMTE_LUMA).image);
    imwrite(P("t-luma_inv.png"), testTrkMat(a, b, TRKMTE_LUMA_INV).image);
    exit(0);
}
extern "C" {
    void CCPLUS_TEST(const char* _opos) {
        opos = _opos;
        pthread_join(ParallelExecutor::runInNewThread([] () {
            void* ctx = createGLContext();
            initGL();
            testMe();
            destroyGLContext(ctx);
        }), 0);
    }
}
// ----

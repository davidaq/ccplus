#include "global.hpp"
#include "platform.hpp"
#include "ccplus.hpp"
#include "context.hpp"
#include "footage-collector.hpp"
#include "composition.hpp"
#include "video-encoder.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "profile.hpp"
#include "parallel-executor.hpp"

pthread_t render_thread = 0;
bool continueRunning = false;

void CCPlus::go(const std::string& tmlPath, const std::string& outputPath, int fps) {
    initContext(tmlPath, outputPath, fps);
    render();
    waitRender();
    encode();
    releaseContext();
    profileFlush;
}

void CCPlus::initContext(const std::string& tmlPath, const std::string& outputPath, int fps) {
    Context::getContext()->begin(tmlPath, outputPath, fps);
}

void CCPlus::releaseContext() {
    continueRunning = false;
    waitRender();
    Context* ctx = Context::getContext();
    ctx->end();
    render_thread = 0;
}

// test
#include "filter.hpp"
#include "platform.hpp"
using namespace CCPlus;
GPUFrame imreadAsset(const std::string& path) {
    cv::Mat mat = readAsset(path.c_str());
    mat = cv::imdecode(mat, CV_LOAD_IMAGE_UNCHANGED);
    mat3to4(mat);
    Frame frame;
    frame.image = mat;
    GPUFrame ret = GPUFrameCache::alloc(mat.cols, mat.rows);
    ret->load(frame);
    return ret;
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
    return Context::getContext()->getStoragePath(p);
}

static inline Frame testMerge(GPUFrame a, GPUFrame b, BlendMode mode) {
    return mergeFrame(a, b, mode)->toCPU();
}

static inline Frame testTrkMat(GPUFrame a, GPUFrame b, TrackMatteMode mode) {
    return trackMatte(a, b, mode)->toCPU();
}

void testME() {
    L() << "TEST HERE!!!!!!";
    Context* ctx = Context::getContext();
    imwrite(P("load.png"), imreadAsset("test/res/test1.jpg")->toCPU().image);

    imwrite(P("transform.png"), testFilter("test/res/test1.jpg", "transform",
                {250, 280, 0, 0, 0, 0, 1, 1, 1, 0, 0, 90}, 500, 500).image);
    imwrite(P("mask.png"), testFilter("test/res/test2.jpg", "mask",
                {50, 50, 300, 100, 325, 100, 350, 50, 375, 100, 400, 100, 350, 300}, 640, 852).image);
    imwrite(P("gaussian.png"), testFilter("test/res/test2.jpg", "gaussian",
                {7, 1}, 640, 852).image);
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
// ----
void CCPlus::render() {
    if (render_thread) {
        log(logERROR) << "Another render context is currently in use! Aborted.";
        return;
    }
    if (!Context::getContext()->isActive()) {
        log(logERROR) << "Context hasn't been initialized! Aborted rendering.";
        return;
    }
    continueRunning = true;
    render_thread = ParallelExecutor::runInNewThread([] () {
        Context* ctx = Context::getContext();
        createGLContext();
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        ctx->collector->limit = 10;
        //ctx->collector->prepare();
        float delta = 1.0f / ctx->fps;
        float duration = ctx->mainComposition->getDuration();
        int fn = 0;
        GPUFrame blackBackground = GPUFrameCache::alloc(
                ctx->mainComposition->width, 
                ctx->mainComposition->height);
        blackBackground->bindFBO();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 0);
        // test 
        testME();
        // ---- 

        //for (float i = 0; i <= duration; i += delta) {
        //    if (!continueRunning) {
        //        log(logINFO) << "----Rendering process is terminated!---";
        //        return;
        //    }
        //    while(ctx->collector->finished() < i) {
        //        log(logINFO) << "wait --" << ctx->collector->finished();
        //        ctx->collector->signal.wait();
        //    }
        //    ctx->collector->limit = i + 10;
        //    log(logINFO) << "render frame --" << i;
        //    GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
        //    frame = mergeFrame(blackBackground, frame, DEFAULT);
        //    char buf[20];
        //    sprintf(buf, "%07d.zim", fn++);
        //    frame->toCPU().write(generatePath(ctx->storagePath, buf));
        //    for(auto item = ctx->renderables.begin();
        //            item != ctx->renderables.end(); ) {
        //        Renderable* r = item->second;
        //        if(r && !r->usedFragments.empty() && r->lastAppearTime < i) {
        //            log(logINFO) << "release" << item->first;
        //            r->release();
        //            ctx->renderables.erase(item++);
        //        } else {
        //            item++;
        //        }
        //    }
        //}
    });
}

void CCPlus::waitRender() {
    if (!render_thread) return;
    pthread_join(render_thread, 0);
}

void CCPlus::encode() {
    CCPlus::waitRender();
    Context* ctx = Context::getContext();
    if (!ctx->isActive()) {
        log(logERROR) << "Context hasn't been initialized! Aborted encoding.";
        return;
    }
    VideoEncoder encoder(
            ctx->getStoragePath("result.mp4"),
            ctx->fps);
    float delta = 1.0 / ctx->fps;
    float duration = ctx->mainComposition->getDuration();
    int fn = 0;
    for (float i = 0; i <= duration; i += delta) {
        Frame f;
        char buf[64];
        sprintf(buf, "%07d.zim", fn);
        f.read(generatePath(ctx->storagePath, buf));
        encoder.appendFrame(f);
        fn++;
    }
    encoder.finish();
}

int CCPlus::numberOfFrames() {
    Context* ctx = Context::getContext();
    float d = 1.0 / ctx->fps;
    return ctx->mainComposition->getDuration() / d;
}


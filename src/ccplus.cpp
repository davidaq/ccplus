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

void CCPlus::go(const std::string& tmlPath, const std::string& outputPath, int fps) {
    initContext(tmlPath, outputPath, fps);
    render();
    encode();
    releaseContext();
    profileFlush;
}

void CCPlus::initContext(const std::string& tmlPath, const std::string& outputPath, int fps) {
    Context::getContext()->begin(tmlPath, outputPath, fps);
}

void CCPlus::releaseContext() {
    Context::getContext()->end();
}

void CCPlus::render() {
    Context* ctx = Context::getContext();
    createGLContext();
    ctx->collector->prepare();
    float delta = 1.0f / ctx->fps;
    float duration = ctx->mainComposition->getDuration();
    int fn = 0;
    GPUFrame blackBackground = GPUFrameCache::alloc(ctx->mainComposition->width, ctx->mainComposition->height);
    blackBackground->bindFBO();
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);
    for (float i = 0; i <= duration; i += delta) {
        while(ctx->collector->finished() < i) {
            log(logINFO) << "wait --" << ctx->collector->finished();
            ctx->collector->signal.wait();
        }
        log(logINFO) << "render frame --" << i;
        GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
        frame = mergeFrame(blackBackground, frame, DEFAULT);
        char buf[64];
        sprintf(buf, "%07d.zim", fn++);
        frame->toCPU().write(generatePath(ctx->storagePath, buf));
    }
}

void CCPlus::encode() {
    Context* ctx = Context::getContext();
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


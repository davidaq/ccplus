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
    GPUFrame screen;
    GPUFrame black;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (float i = 0; i <= duration; i += delta) {
        L() << "-- " << i;
        ctx->mainComposition->updateGPUFrame(screen, i);
        if(!black.textureID)
            black.createTexture(screen.width, screen.height);
        black.bindFBO();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 0);
        glEnable(GL_BLEND);
        mergeFrame(screen, screen, NONE);
        glDisable(GL_BLEND);
        char buf[64];
        sprintf(buf, "%07d.zim", fn++);
        black.ext = screen.ext;
        black.toCPU().write(generatePath(ctx->storagePath, buf));
    }
    screen.destroy();
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


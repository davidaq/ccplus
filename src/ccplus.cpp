#include "global.hpp"
#include "ccplus.hpp"
#include "context.hpp"
#include "footage-collector.hpp"
#include "composition.hpp"
#include "video-encoder.hpp"

void CCPlus::go(const std::string& tmlPath, const std::string& outputPath, int fps) {
    initContext(tmlPath, outputPath, fps);
    render();
    encode();
    releaseContext();
}

void CCPlus::initContext(const std::string& tmlPath, const std::string& outputPath, int fps) {
    Context::getContext()->begin(tmlPath, outputPath, fps);
}

void CCPlus::releaseContext() {
    Context::getContext()->end();
}

void CCPlus::render() {
    Context* ctx = Context::getContext();
    ctx->collector->prepare();
    float delta = 1.0f / ctx->fps;
    float duration = ctx->mainComposition->getDuration();
    int fn = 0;
    for (float i = 0; i <= duration; i+=delta) {
        Frame f = ctx->mainComposition->getFrame(i);
        char buf[64];
        sprintf(buf, "%d.zim", fn);
        f.write(generatePath(ctx->storagePath, buf));
        fn++;
    }
}

void CCPlus::encode() {
    // TODO implement
    Context* ctx = Context::getContext();
    VideoEncoder encoder(
            generatePath(ctx->storagePath, ""),
            ctx->fps);
    float delta = 1.0 / ctx->fps;
    float duration = ctx->mainComposition->getDuration();
    int fn = 0;
    for (float i = 0; i <= duration; i+=delta) {
        Frame f;
        char buf[64];
        sprintf(buf, "%d.zim", fn);
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


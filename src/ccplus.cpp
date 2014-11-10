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

using namespace CCPlus;

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
        void* glCtx = createGLContext();
        ctx->collector->limit = 10;
        ctx->collector->prepare();
        float delta = 1.0f / ctx->fps;
        float duration = ctx->mainComposition->getDuration();
        int fn = 0;
        initGL();
        GPUFrame blackBackground = GPUFrameCache::alloc(
                ctx->mainComposition->width, 
                ctx->mainComposition->height);
        blackBackground->bindFBO();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 0);

        for (float i = 0; i <= duration; i += delta) {
            if (!continueRunning) {
                log(logINFO) << "----Rendering process is terminated!---";
                return;
            }
            while(ctx->collector->finished() < i) {
                log(logINFO) << "wait --" << ctx->collector->finished();
                ctx->collector->signal.wait();
            }
            ctx->collector->limit = i + 10;
            log(logINFO) << "render frame --" << i;
            GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
            frame = mergeFrame(blackBackground, frame, DEFAULT);
            char buf[20];
            sprintf(buf, "%07d.zim", fn++);
            frame->toCPU().write(ctx->getStoragePath(buf));
            for(auto item = ctx->renderables.begin();
                    item != ctx->renderables.end(); ) {
                Renderable* r = item->second;
                if(r && !r->usedFragments.empty() && r->lastAppearTime < i) {
                    log(logINFO) << "release" << item->first;
                    r->release();
                    ctx->renderables.erase(item++);
                } else {
                    item++;
                }
            }
        }
        destroyGLContext(glCtx);
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

std::string assetPath;
void CCPlus::setAssetsPath(const std::string& path) {
    assetPath = path;
}

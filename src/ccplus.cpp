#include <limits>

#include "global.hpp"
#include "ccplus.hpp"
#include "utils.hpp"
#include "video-encoder.hpp"

// Tweak
#include "image-renderable.hpp"

//using namespace CCPlus;

struct UserContext {
    CCPlus::Context* ctx;
    CCPlus::Composition* mainComp;
};

void* CCPlus::initContext(const char* tmlPath, const char* storagePath, int fps) {
    log(logINFO) << "Loading tml file......";
    UserContext* ret = new UserContext;
    ret->ctx = new CCPlus::Context(storagePath, fps);
    CCPlus::TMLReader reader(ret->ctx);
    ret->mainComp = reader.read(tmlPath);
    return (void*) ret;
}

void CCPlus::releaseContext(void* ctxHandle) {
    if(!ctxHandle)
        return;
    UserContext* uCtx = (UserContext*) ctxHandle;
    delete uCtx->ctx;
    delete uCtx;
}

void CCPlus::renderPart(void* ctxHandle, float start, float length) {
    if(!ctxHandle)
        return;
    log(logINFO) << "Start rendering......";
    UserContext* uCtx = (UserContext*) ctxHandle;
    if(length < 0)
        length = uCtx->mainComp->getDuration();
    uCtx->mainComp->setForceRenderToFile(true);
    std::vector<CCPlus::CompositionDependency> deps = uCtx->mainComp->fullOrderedDependency(start, length);
    float total_time = 0;
    for (auto& dep : deps) {
        if (dynamic_cast<ImageRenderable*>(dep.renderable))
            continue;
        total_time += std::min(dep.to - dep.from, dep.renderable->getDuration());
    }
    float done_time = 0;
    for (auto& dep : deps) {
        dep.renderable->render(dep.from, dep.to);
        if (!dynamic_cast<ImageRenderable*>(dep.renderable)) {
            done_time += std::min(dep.to - dep.from, dep.renderable->getDuration());
            log(logINFO) << "Rendering progress: " << done_time * 100.0 / total_time << "%";
        }
    }
}

void CCPlus::encodeVideo(void* ctxHandle, float start, float length) {
    if(!ctxHandle)
        return;
    log(logINFO) << "Encoding video......";
    UserContext* uCtx = (UserContext*) ctxHandle;
    if(length < 0)
        length = uCtx->mainComp->getDuration();
    float inter = 1.0 / uCtx->ctx->getFPS();
    VideoEncoder encoder(uCtx->ctx->getStoragePath() + "/result.mp4", uCtx->ctx->getFPS());
    for (float i = 0.0; i < length; i += inter) {
        float t = start + i;
        CCPlus::Frame img = uCtx->mainComp->getFrame(t);
        encoder.appendFrame(img);
    }
    encoder.finish();

    log(logINFO) << "---Done!---";
}

void CCPlus::go(
        const std::string& tmlpath,
        const std::string& storagePath, 
        float start,
        float length,
        int fps) {

    void* ctx = initContext(tmlpath.c_str(), storagePath.c_str(), fps);
    profile(rendering) {
        renderPart(ctx, start, length);
    }
    profile(encoding) {
        encodeVideo(ctx, start, length);
    }
    profileFlush;
}

extern "C" {
#include "ccplus.h"
    void* CCPlusInitContext(const char* tmlPath, const char* storagePath, int fps) {
        return CCPlus::initContext(tmlPath, storagePath, fps);
    }
    void CCPlusReleaseContext(void* ctxHandle) {
       CCPlus::releaseContext(ctxHandle);
    }
    void CCPlusRenderPart(void* ctxHandle, float start, float length) {
        CCPlus::renderPart(ctxHandle, start, length);
    }
    void CCPlusEncodeVideo(void* ctxHandle, float start, float length) {
        CCPlus::encodeVideo(ctxHandle, start, length);
    }
}

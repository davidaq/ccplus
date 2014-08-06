#include "global.hpp"
#include "ccplus.hpp"
#include "utils.hpp"
#include "video-encoder.hpp"

//using namespace CCPlus;

struct UserContext {
    CCPlus::Context* ctx;
    CCPlus::Composition* mainComp;
};

void* CCPlus::initContext(const char* tmlPath, const char* storagePath, int fps) {
    log(logDEBUG) << "Loading tml file......";
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
    log(logDEBUG) << "Start rendering......";
    UserContext* uCtx = (UserContext*) ctxHandle;
    if(length < 0)
        length = uCtx->mainComp->getDuration();
    std::vector<CCPlus::CompositionDependency> deps = uCtx->mainComp->fullOrderedDependency(start, length);
    for (auto& dep : deps) {
        dep.renderable->render(dep.from, dep.to);
    }
}

void CCPlus::encodeVideo(void* ctxHandle, float start, float length) {
    if(!ctxHandle)
        return;
    log(logDEBUG) << "Encoding video......";
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

    log(logDEBUG) << "---Done!---";
}

void CCPlus::go(
        const std::string& tmlpath,
        const std::string& storagePath, 
        float start,
        float length,
        int fps) {

    void* ctx = initContext(tmlpath.c_str(), storagePath.c_str(), fps);
    renderPart(ctx, start, length);
    encodeVideo(ctx, start, length);
}

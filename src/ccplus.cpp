#include <limits>
#include <ctime>

#include "global.hpp"
#include "ccplus.hpp"
#include "utils.hpp"
#include "video-encoder.hpp"
#include "parallel-executor.hpp"

// Tweak
#include "image-renderable.hpp"
#include "composition.hpp"

//using namespace CCPlus;

struct UserContext {
    CCPlus::Context* ctx;
    CCPlus::Composition* mainComp;
};

void* CCPlus::initContext(const char* tmlPath, 
        const char* storagePath, int fps) {
    log(logINFO) << "Loading tml file......";
    UserContext* ret = new UserContext;
    ret->ctx = CCPlus::Context::getInstance();
    ret->ctx->init(storagePath, fps);
    CCPlus::TMLReader reader;
    ret->mainComp = reader.read(tmlPath);

    // Init rand seed
    srand(std::time(0));

    return (void*) ret;
}

void CCPlus::releaseContext(void* ctxHandle) {
    if(!ctxHandle)
        return;
    UserContext* uCtx = (UserContext*) ctxHandle;
    uCtx->ctx->releaseMemory();
    delete uCtx;
}

void CCPlus::freeMemory(void* ctxHandle) {
    if(!ctxHandle)
        return;
    UserContext* uCtx = (UserContext*) ctxHandle;
    uCtx->ctx->releaseMemory();
    log(logINFO) << "Released memory";
}

void CCPlus::renderPart(void* ctxHandle, float start, float length) {
    if(!ctxHandle)
        return;
    UserContext* uCtx = (UserContext*) ctxHandle;
    while (start > uCtx->mainComp->getDuration())
        start -= uCtx->mainComp->getDuration();
    while (start < 0)
        start += uCtx->mainComp->getDuration();
    if(length < 0)
        length = uCtx->mainComp->getDuration() - start;
    log(logINFO) << "Start rendering...... start:" << start << " length:" << length;
    uCtx->mainComp->setForceRenderToFile(true);
    std::vector<CCPlus::CompositionDependency> deps = uCtx->mainComp->fullOrderedDependency(start, start + length);
    // Clear previous used memory
    //uCtx->ctx->releaseMemory();

    float total_time = 0;
    for (auto& dep : deps) {
        if (dynamic_cast<ImageRenderable*>(dep.renderable))
            continue;
        total_time += std::min(dep.to - dep.from, dep.renderable->getDuration());
    }
    float done_time = 0;
    /*
     * Render non-composition renderable parallelly
     */
    ParallelExecutor* exec = new ParallelExecutor(CCPlus::CONCURRENT_THREAD);
    for (auto& dep : deps) {
        if (dynamic_cast<Composition*>(dep.renderable))
            continue;
        auto task = [&dep, &done_time, &total_time] () {
            Profiler* p = new Profiler("Renderable__" + dep.renderable->getName());
            dep.renderable->render(dep.from, dep.to - dep.from);
            delete p;
            if (!dynamic_cast<ImageRenderable*>(dep.renderable)) {
                done_time += std::min(dep.to - dep.from, dep.renderable->getDuration());
                log(logINFO) << "Rendering progress: " << done_time * 100.0 / total_time << "%";
            }
        };
        exec->execute(task);
    }
    delete exec;

    /*
     * Only render Composition
     */
    for (auto& dep : deps) {
        if (!dynamic_cast<Composition*>(dep.renderable))
            continue;
        Profiler* p = new Profiler("Renderable__" + dep.renderable->getName());
        dep.renderable->render(dep.from, dep.to - dep.from);
        delete p;
        done_time += std::min(dep.to - dep.from, dep.renderable->getDuration());
        log(logINFO) << "Rendering progress: " << done_time * 100.0 / total_time << "%";
    }
    profileFlush;
}

void CCPlus::encodeVideo(void* ctxHandle, float start, float length) {
    if(!ctxHandle)
        return;
    log(logINFO) << "Encoding video......";
    UserContext* uCtx = (UserContext*) ctxHandle;
    // TODO: duplicate code with renderPart
    while (start > uCtx->mainComp->getDuration())
        start -= uCtx->mainComp->getDuration();
    while (start < 0)
        start += uCtx->mainComp->getDuration();
    if(length < 0)
        length = uCtx->mainComp->getDuration() - start;
    float inter = 1.0 / uCtx->ctx->getFPS();
    VideoEncoder encoder(uCtx->ctx->getStoragePath() + "/result.mp4", uCtx->ctx->getFPS());
    for (float i = 0.0; i < length; i += inter) {
        float t = start + i;
        CCPlus::Frame img = uCtx->mainComp->getFrame(t);
        img.setBlackBackground();
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
    log(logINFO) << "Finish rendering...";
    log(logINFO) << "\tZIM path prefix: " << getZIMPath(ctx);
    log(logINFO) << "\tZIM number: " << numberOfZIM(ctx);
    profile(encoding) {
        encodeVideo(ctx, start, length);
    }
    releaseContext(ctx);
    profileFlush;
}

const std::string CCPlus::getZIMPath(void* ctx) {
    UserContext* uCtx = (UserContext*) ctx;
    if (uCtx->mainComp)
        return uCtx->mainComp->getPrefix();
    log(logERROR) << 
        "Accessed Main Composition before its initialization";
    return "";
}

int CCPlus::numberOfZIM(void* ctx) {
    UserContext* uCtx = (UserContext*) ctx;
    if (uCtx->mainComp)
        return uCtx->mainComp->getTotalNumberOfFrame();
    log(logERROR) << 
        "Accessed Main Composition before its initialization";
    return 0;
}

bool CCPlus::finishedFrame(void* ctx, int frame) {
    UserContext* uCtx = (UserContext*) ctx;
    if (uCtx->mainComp)
        return uCtx->mainComp->finished(frame);
    log(logERROR) << 
        "Accessed Main Composition before its initialization";
    return false;
}

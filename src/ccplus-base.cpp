#include "global.hpp"
#include "ccplus-base.hpp"
#include "ccplus.hpp"
#include "context.hpp"
#include "footage-collector.hpp"
#include "composition.hpp"
#include "video-encoder.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "profile.hpp"

#ifdef TRACE_RAM_USAGE
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h> 
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif

using namespace CCPlus;


bool CCPlus::continueRunning = false;
int CCPlus::renderProgress = 0;

void traceRam() {
#ifdef TRACE_RAM_USAGE
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (KERN_SUCCESS != task_info(mach_task_self(),
                TASK_BASIC_INFO, (task_info_t)&t_info, 
                &t_info_count)) {
    } else {
        L() << "RAM used:" << (int64_t)t_info.resident_size;
    }
#endif
}

void CCPlus::stop() {
    continueRunning = false;
}

void CCPlus::initContext(const std::string& tmlPath) {
    if (!continueRunning) return;
    Context::getContext()->begin(tmlPath);
}

void CCPlus::releaseContext() {
    Context* ctx = Context::getContext();
    ctx->end();
    profileFlush;
}

typedef void* (*BeginFunc)(void);
typedef void (*WriteFunc)(const Frame&, int, void* ctx);
typedef void (*FinishFunc)(void*);

void renderAs(BeginFunc beginFunc, WriteFunc writeFuc, FinishFunc finishFunc) {
    if (!continueRunning) return;
    Context* ctx = Context::getContext();
    ctx->collector->prepare();
    profileBegin(GL_INIT);
    void* glCtx = createGLContext();
    initGL();
    profileEnd(GL_INIT);
    float delta = 1.0f / frameRate;
    float duration = ctx->mainComposition->getDuration();

    ctx->mainComposition->transparent = false;

    int fn = 0;

    void* writeCtx = beginFunc ? beginFunc() : 0;
    for (float i = 0; i <= duration; i += delta) {
        renderProgress = (i * 98 / duration) + 1;
        while(continueRunning && ctx->collector->finished() <= i + 0.1) {
            log(logINFO) << "wait --" << ctx->collector->finished();
            usleep(500000);
        }
        if (!continueRunning) {
            log(logINFO) << "----Rendering process is terminated!---";
            return;
        }
        GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
        if(writeFuc) {
            Frame cpu_frame = frame->toCPU();
            if (i + delta > duration) {
                cpu_frame.eov = true;
            }
            writeFuc(cpu_frame, fn++, writeCtx);
        }
        if(fn & 1) {
            log(logINFO) << "render frame --" << i << ':' << renderProgress << '%';
            traceRam();

            ctx->collector->clean(i);
        }
    }
    if(finishFunc)
        finishFunc(writeCtx);
    destroyGLContext(glCtx);

    renderProgress = 100;
}

void* beginVideo() {
    Context* ctx = Context::getContext();
    std::string outfile = outputPath;
    if(!stringEndsWith(outfile, ".mp4")) {
        outfile = Context::getContext()->getStoragePath("result.mp4");
    }
    return new VideoEncoder(outfile, frameRate, 
            nearestPOT(ctx->mainComposition->width), nearestPOT(ctx->mainComposition->height));
}

void writeVideo(const Frame& frame, int fn, void* ctx) {
    VideoEncoder* encoder = static_cast<VideoEncoder*>(ctx);
    encoder->appendFrame(frame);
}

void finishVideo(void* ctx) {
    VideoEncoder* encoder = static_cast<VideoEncoder*>(ctx);
    encoder->finish();
    delete encoder;
}

void writePreview(const Frame& frame, int fn, void* ctx) {
    char buf[20];
    sprintf(buf, "%07d.zim", fn);
    frame.write(Context::getContext()->getStoragePath(buf));
}

void writeEOF(void* ctx) {
    std::string path = Context::getContext()->getStoragePath("eov.zim");
    spit(path, "I'm an extremly bored EOV -- End Of Video.");
}

void CCPlus::render() {
    if(renderMode == FINAL_MODE) {
        renderAs(beginVideo, writeVideo, finishVideo);
    } else {
        renderAs(0, writePreview, writeEOF);
    }
}


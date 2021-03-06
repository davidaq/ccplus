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

bool CCPlus::appPaused = false;
bool CCPlus::continueRunning = false;
int CCPlus::renderProgress = 0;

static inline void traceRam() {
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

void CCPlus::initContext(const std::string& tmlPath, const std::string& footageDir) {
    if (!continueRunning) return;
    Context::getContext()->begin(tmlPath, footageDir);
}

void CCPlus::releaseContext() {
    Context* ctx = Context::getContext();
    ctx->end();
    profileFlush;
}

typedef void* (*BeginFunc)(void);
typedef void (*WriteFunc)(const Frame&, int, void* ctx);
typedef void (*FinishFunc)(void*, bool isFinish);

static inline float bgmItp(Context* ctx, float i) {
    float low = -1, high = -1;
    float low_time = -1, high_time = -1;
    for (int j = 0; j < ctx->bgmVolumes.size(); j++) {
        float t = ctx->bgmVolumes[j].first;
        float v = ctx->bgmVolumes[j].second;
        if (std::abs(v - i) < 0.01) {
            return v;
        }
        if (t < i) {
            low_time = t;
            low = v;
        } else if (t > i) {
            high_time = t;
            high = v;
        }
        if (high != -1 && low != -1) break;
    }
    if (high == -1 || low == -1) return 1.0f;
    return (i - low_time) / (high_time - low_time) * (high - low) + low;
};
void renderAs(BeginFunc beginFunc, WriteFunc writeFuc, FinishFunc finishFunc) {
    if (!continueRunning) return;
    Context* ctx = Context::getContext();
    ctx->collector->prepare();
    void* glCtx = createGLContext();
    initGL();
    float delta = 1.0f / frameRate;
    float duration = ctx->mainComposition->getDuration();

    ctx->mainComposition->transparent = false;

    int fn = 0;

    void* writeCtx = beginFunc ? beginFunc() : 0;
    ScopeHelper onEnd([writeCtx, &finishFunc, glCtx]{
        if(finishFunc) {
            finishFunc(writeCtx, false);
        }
        destroyGLContext(glCtx);
    });
    for (float i = 0; i <= duration; i += delta) {
        renderProgress = (i * 98 / duration) + 1;
        while(continueRunning && ctx->collector->finished() <= i + 0.1) {
            log(logINFO) << "wait --" << ctx->collector->finished();
            ctx->collector->signal.wait();
        }
        if (!continueRunning) {
            log(logINFO) << "----Rendering process is terminated!---";
            return;
        }
        try {
            GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
            if(writeFuc) {
                Frame cpu_frame = frame->toCPU();
                if (i + delta > duration) {
                    cpu_frame.eov = true;
                }
                cpu_frame.bgmVolume = bgmItp(ctx, i);
                writeFuc(cpu_frame, fn++, writeCtx);
            }
            if(fn & 1) {
                log(logINFO) << "render --" << i << ':' << renderProgress << '%';
                traceRam();
                ctx->collector->clean(i);
            }
        } catch(int e) {
            L() << "Render interupted";
            if(e == 0xfff) {
                L() << "Render paused";
                GPUFrameCache::clear();
                do {
                    sleep(1);
                } while(appPaused && continueRunning);
                if(continueRunning) {
                    glCtx = createGLContext();
                    initGL();
                    i -= delta;
                }
            }
        }
    }
    if(finishFunc)
        finishFunc(writeCtx, true);
    finishFunc = 0;

    renderProgress = 100;
}

void* beginVideo() {
    Context* ctx = Context::getContext();
    std::string outfile = outputPath;
    if(!stringEndsWith(outfile, ".mp4")) {
        outfile = Context::getContext()->getStoragePath("result.mp4");
    }
    float ratio = ctx->mainComposition->width * 1.0 / ctx->mainComposition->height;
    int w = nearestPOT(ctx->mainComposition->width);
    int h = (int)(w / ratio);
    return new VideoEncoder(outfile, frameRate, w, h);
}

void writeVideo(const Frame& frame, int fn, void* ctx) {
    VideoEncoder* encoder = static_cast<VideoEncoder*>(ctx);
    if(!frame.image.empty() && (frame.image.cols != encoder->getWidth() || frame.image.rows != encoder->getHeight())) {
        cv::resize(frame.image, frame.image, cv::Size(encoder->getWidth(), encoder->getHeight()));
    }
    encoder->appendFrame(frame);
}

void finishVideo(void* ctx, bool isFinish) {
    VideoEncoder* encoder = static_cast<VideoEncoder*>(ctx);
    encoder->finish();
    delete encoder;
}

void writePreview(const Frame& frame, int fn, void* ctx) {
    char buf[20];
    sprintf(buf, "%07d.zim", fn);
    frame.write(Context::getContext()->getStoragePath(buf));
}

void writeEOF(void* ctx, bool isFinish) {
    if(isFinish) {
        std::string path = Context::getContext()->getStoragePath("eov.zim");
        spit(path, "I'm an extremly bored EOV -- End Of Video.");
    }
}

void CCPlus::render() {
    if(renderMode == FINAL_MODE) {
        renderAs(beginVideo, writeVideo, finishVideo);
    } else {
        renderAs(0, writePreview, writeEOF);
    }
}


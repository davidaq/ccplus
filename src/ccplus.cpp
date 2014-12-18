#include "global.hpp"
#include "ccplus.hpp"
#include "context.hpp"
#include "footage-collector.hpp"
#include "composition.hpp"
#include "video-encoder.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "profile.hpp"
#include "parallel-executor.hpp"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>

#ifdef TRACE_RAM_USAGE
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h> 
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif

using namespace CCPlus;

pthread_t render_thread = 0;
bool continueRunning = false;

// The life-cycle of these two object are managed by rendering thread
pthread_mutex_t renderLock;
RenderTarget* activeTarget = 0;
RenderTarget* pendingTarget = 0;

RenderTarget::RenderTarget() {
    uuid = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
}

RenderTarget::~RenderTarget() {
    this->stop();
    this->waitFinish();
}

void RenderTarget::waitFinish() {
    pthread_mutex_lock(&renderLock);
    if ((activeTarget && *activeTarget == *this) || 
        (pendingTarget && *pendingTarget == *this)) {
        pthread_join(render_thread, NULL);
    }
    pthread_mutex_unlock(&renderLock);
}

void RenderTarget::stop() {
    pthread_mutex_lock(&renderLock);
    if (activeTarget && *activeTarget == *this) {
        continueRunning = false;
    }
    pthread_mutex_unlock(&renderLock);
    // Lazy stop
    this->stopped = true;
}

void initContext(const std::string& tmlPath) {
    if (!continueRunning) return;
    Context::getContext()->begin(tmlPath);
}

void releaseContext() {
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
    ctx->collector->limit = 5;
    ctx->collector->prepare();
    void* glCtx = createGLContext();
    initGL();
    float delta = 1.0f / frameRate;
    float duration = ctx->mainComposition->getDuration();

    ctx->mainComposition->transparent = false;

    int fn = 0;

    void* writeCtx = beginFunc ? beginFunc() : 0;
    for (float i = 0; i <= duration; i += delta) {
        activeTarget->progress = (i * 98 / duration) + 1;
        while(continueRunning && ctx->collector->finished() < i) {
            log(logINFO) << "wait --" << ctx->collector->finished();
            usleep(500000);
        }
        if (!continueRunning) {
            log(logINFO) << "----Rendering process is terminated!---";
            return;
        }
        ctx->collector->limit = i + (renderMode == PREVIEW_MODE ? 7 : 5);
        GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
        if(writeFuc) {
            Frame cpu_frame = frame->toCPU();
            if (i + delta > duration) {
                cpu_frame.eov = true;
            }
            writeFuc(cpu_frame, fn++, writeCtx);
        }
        if(fn & 1) {
            log(logINFO) << "render frame --" << i << ':' << activeTarget->progress << '%';

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
    }
    if(finishFunc)
        finishFunc(writeCtx);
    destroyGLContext(glCtx);

    activeTarget->progress = 100;
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

void render() {
    if(renderMode == FINAL_MODE) {
        renderAs(beginVideo, writeVideo, finishVideo);
    } else {
        renderAs(0, writePreview, writeEOF);
    }
}

void CCPlus::go(const std::string& tmlPath) {
    RenderTarget* target = new RenderTarget();
    target->tmlPath = tmlPath;
    go(target);
    target->waitFinish();
    delete target;
}

void CCPlus::go(RenderTarget* target) {
    pthread_mutex_lock(&renderLock);
    if (activeTarget && *activeTarget == *target) {
        return;
    }
    if (pendingTarget && *pendingTarget == *target) {
        return;
    }
    pendingTarget = target;
    if (activeTarget) {
        activeTarget->stop();
    }
    pthread_mutex_unlock(&renderLock);

    if (!render_thread) {
        /*************************************
         * Only this thread will be able to modify @activeTarget
         ************************************/
        render_thread = ParallelExecutor::runInNewThread([] () {
            while (true) {
                pthread_mutex_lock(&renderLock);
                if (!pendingTarget || activeTarget) {
                    pthread_mutex_unlock(&renderLock);
                    break;
                }
                activeTarget = pendingTarget;
                pendingTarget = 0;
                pthread_mutex_unlock(&renderLock);

                continueRunning = !activeTarget->stopped;
                initContext(activeTarget->tmlPath);
                // Keep GL context in a seprated thread context
                // May the ugliness be our POWER!!!!
                pthread_t tmp_thread = ParallelExecutor::runInNewThread([] () {
                    render();
                });
                pthread_join(tmp_thread, NULL);
                releaseContext();
                continueRunning = false;

                pthread_mutex_lock(&renderLock);
                activeTarget = 0;
                pthread_mutex_unlock(&renderLock);
            }
            render_thread = 0;
        });
    }
}
